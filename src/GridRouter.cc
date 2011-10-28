#include "GridRouter.hh"
#include <iostream>
#include <cassert>
#include <cmath>
#include <set>
#include <algorithm>
#include "CommTable.hh"
#include "Grid3DStencil.hh"
#include "DomainInfo.hh"
#include "Vector.hh"
#include "IndexToVector.hh"

using namespace std;

GridRouter::GridRouter(vector<Long64>& gid, int nx, int ny, int nz, MPI_Comm comm)
: comm_(comm)
{
   int nTasks, myRank;
   MPI_Comm_size(comm_, &nTasks);
   MPI_Comm_rank(comm_, &myRank);  

   double deltaR = 2.0;
  

   DomainInfo myInfo(gid, nx, ny, nz);
   Vector myCenter = myInfo.center();
   double myRadius = myInfo.radius();
  
   // distribute process centers and radii to all tasks
   vector<DomainInfo> dInfo(nTasks, myInfo);
   int dSize = sizeof(DomainInfo);
   MPI_Allgather(&myInfo, dSize, MPI_BYTE, &dInfo[0], dSize, MPI_BYTE, comm);

   // compute all tasks which overlap with this process
   vector<int> myNbrs;
   { //scope
      for (int ii=0; ii<nTasks; ++ii)
      {
	 if (ii == myRank)
	    continue;
	 double rij = 0.0;
	 for (int jj=0; jj<3; ++jj)
	 {
	    double xij = dInfo[ii].center()[jj] - myCenter[jj];
	    rij += xij*xij;
	 }
	 assert(rij > 0.0);
	 rij = sqrt(rij);
	 // this process is a potential neighbor, add it to the list
	 if (rij <= myRadius + dInfo[ii].radius() + deltaR)
	    myNbrs.push_back(ii);
      }
   } //scope

   // Get a list of all of the cells I might possibly need on this task.
   // This list might include non-tissue cells since we have no way of
   // telling. 
   vector<Long64> neededCells;
   {//scope
      set<Long64> stencilGids;
      set<Long64> myGids;
      for (unsigned ii=0; ii<gid.size(); ++ii)
      {
	 myGids.insert(gid[ii]);
	 Grid3DStencil stencil(gid[ii], nx, ny, nz);
	 for (int jj=0; jj<stencil.nStencil(); ++jj)
	    stencilGids.insert(stencil[jj]);
      }
      set_difference(stencilGids.begin(), stencilGids.end(),
		     myGids.begin(), myGids.end(), 
		     back_inserter(neededCells));
   } //scope

   
   vector<Long64> sendBuf;  
   vector<int> sendOffset;
   { //scope
      sendBuf.reserve(2*gid.size());
      sendOffset.reserve(myNbrs.size() + 1);
      sendOffset.push_back(0);
      IndexToVector indexToVector(nx, ny, nz);
      vector<Vector> neededVectors;
      neededVectors.reserve(neededCells.size());
      for (unsigned ii=0; ii<neededCells.size(); ++ii)
	 neededVectors.push_back(indexToVector(neededCells[ii]));
     
      for (unsigned ii=0; ii<myNbrs.size(); ++ii)
      {
	 Vector ri = dInfo[myNbrs[ii]].center();
	 double rMax2 = dInfo[myNbrs[ii]].radius();
	 rMax2 *= rMax2;
	 for (unsigned jj=0; jj<neededCells.size(); ++jj)
	 {
	    Vector rij = ri - neededVectors[jj];
	    if ( dot(rij, rij)  < rMax2)
	       sendBuf.push_back(neededCells[jj]);
	 }
	 sendOffset.push_back(sendBuf.size());
      }
   } //scope

   
   // send request size to all neighbors
   vector<int> recvOffset(myNbrs.size()+1);
   { //scope
      recvOffset[0] = 0;
      const int tag = 78539;
      int nNbrs = myNbrs.size();
      MPI_Request sendReq[nNbrs];
      MPI_Request recvReq[nNbrs];
      for (int ii=0; ii<nNbrs; ++ii)
      {
	 int source = myNbrs[ii];
	 MPI_Irecv(&recvOffset[ii+1], 1, MPI_INT, source, tag, comm_, recvReq+ii);
      }  
      vector<int> buf(nNbrs);
      for (int ii=0; ii<nNbrs; ++ii)
      {
	 int dest = myNbrs[ii];
	 buf[ii] = sendOffset[ii+1]-sendOffset[ii];
	 MPI_Isend(&buf[ii], 1, MPI_INT, dest, tag, comm_, sendReq+ii);
      }  
      MPI_Waitall(nNbrs, sendReq, MPI_STATUSES_IGNORE);
      MPI_Waitall(nNbrs, recvReq, MPI_STATUSES_IGNORE);

      for (int ii=0; ii<myNbrs.size(); ++ii)
	 recvOffset[ii+1] += recvOffset[ii];
   } //scope
  
   // send cell requests to neighbors
   vector<Long64> recvBuf(recvOffset.back());
   {
      int nNbrs = myNbrs.size();
      MPI_Request sendReq[nNbrs];
      MPI_Request recvReq[nNbrs];
      const int tag = 78540;
      for (unsigned ii=0; ii<nNbrs; ++ii)
      {
	 int source = myNbrs[ii];
	 int nRecv = recvOffset[ii+1] - recvOffset[ii];
	 MPI_Irecv(&recvBuf[recvOffset[ii]], nRecv, MPI_LONG_LONG, source , tag ,comm_, recvReq+ii);
      }  
     
      for (int ii=0; ii<nNbrs; ++ii)
      {
	 int dest = myNbrs[ii];
	 int nSend = sendOffset[ii+1]-sendOffset[ii];
	 MPI_Isend(&sendBuf[sendOffset[ii]], nSend, MPI_LONG_LONG, dest, tag, comm_, sendReq+ii);
      }  
      MPI_Waitall(nNbrs, sendReq, MPI_STATUSES_IGNORE);
      MPI_Waitall(nNbrs, recvReq, MPI_STATUSES_IGNORE);
   }
  

   sendRank_.clear();
   sendMap_.clear();
   sendOffset_.clear();
   sendOffset_.push_back(0);
   for (unsigned ii=0; ii<myNbrs.size(); ++ii)
   {
      vector<Long64>::const_iterator first = recvBuf.begin() + recvOffset[ii];
      vector<Long64>::const_iterator last =  recvBuf.begin() + recvOffset[ii+1];
      for (unsigned jj=0; jj<gid.size(); ++jj)
      {
	 if (binary_search(first, last, gid[jj]))
	    sendMap_.push_back(jj);
      }
      if (sendMap_.size() > sendOffset_.back())
      {
	 sendRank_.push_back(myNbrs[ii]);
	 sendOffset_.push_back(sendMap_.size());
      }
     
   }
}

CommTable GridRouter::commTable() const
{
   assert(sendOffset_.size() == sendRank_.size() + 1);
   return CommTable(sendRank_, sendOffset_, comm_);
}


