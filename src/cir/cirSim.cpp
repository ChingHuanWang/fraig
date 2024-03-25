/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/

void 
CirMgr::initFecGrps()
{
   if (!_fecGrps)
      _fecGrps = new vector<FecGrp>;
   if (_fecGrps->size() == 0) {
      _fecGrps->push_back(_dfsList);
      (*_fecGrps)[0].add(0);
   }
}

void
CirMgr::setSimResult()
{
   for (CirPiGate* pi : _piList) 
      pi->setSim0(0);
}

void
CirMgr::topoCalSim()
{
   for (CirGate* gate : _dfsList) {
      if (gate->isAig()) {
         gate->setSim0(gate->isIn0Inv() ? ~gate->getIn0Gate()->getSimResult() : gate->getIn0Gate()->getSimResult());
         gate->setSim1(gate->isIn1Inv() ? ~gate->getIn1Gate()->getSimResult() : gate->getIn1Gate()->getSimResult());
      }
      else if (gate->isPo()) {
         gate->setSim0(gate->isIn0Inv() ? ~gate->getIn0Gate()->getSimResult() : gate->getIn0Gate()->getSimResult());
      }
   }
}

void
CirMgr::simulate(size_t bitNum)
{
   if (bitNum == 0)
      return;
   topoCalSim();
   collectFecGrps();
   if (_simLog) 
      writeLog(bitNum); 
   setSimResult();
}

void 
CirMgr::writeLog(size_t wordLen)
{
   
   for (size_t i = 0; i < wordLen; i++) {
      for (size_t j = 0; j < _piList.size(); j++) 
         *_simLog << _piList[j]->getSimBit(i);
      
      *_simLog << " ";

      for (CirPoGate* gate: _poList)
         *_simLog << gate->getSimBit(i);

      *_simLog << endl;
   }
}


void 
CirMgr::collectFecGrps()
{
   initFecGrps();
   vector<FecGrp>* newFecGrps = new vector<FecGrp>;
   for (FecGrp& grp : *_fecGrps) {
      unordered_map<size_t, FecGrp, HashInt> fecGrpsMap;
      for (size_t i = 0; i < grp.size(); i++) {
         size_t gateId = grp[i]/2;
         CirGate* gate = _idList[gateId];
         size_t simVal = gate->getSimResult();
         unordered_map<size_t, FecGrp, HashInt>::iterator tmp;
         if ((tmp = fecGrpsMap.find(simVal)) != fecGrpsMap.end())
            tmp->second.add(gateId*2);
         else if ((tmp = fecGrpsMap.find(~simVal)) != fecGrpsMap.end())
            tmp->second.add(gateId*2+1);
         else {
            FecGrp& tmpGrp = fecGrpsMap[simVal];
            tmpGrp.add(gateId*2);
         }
      }  
      
      
      for (auto& tmpGrp : fecGrpsMap) {
         if (tmpGrp.second.size() > 1) {
            tmpGrp.second.sortGrp();  
            newFecGrps->push_back(tmpGrp.second);
         }
      }
   }
   delete _fecGrps;
   _fecGrps = newFecGrps;
   // cout << "grps size = " << _fecGrps->size() << endl;
}


void
CirMgr::printFECPairs() const
{
   if (!_fecGrps)
      return;
   size_t grpNum = 0;
   for (const FecGrp& grp : *_fecGrps) {
      cout << "[" << grpNum << "] ";
      for (size_t i = 0; i < grp.size(); i++) {
         cout << (grp[i] % 2 == 1 ? "!" : "")
              << grp[i] / 2 << " ";
      }
      cout << '\n';
      grpNum++;
   }
}


bool
CirMgr::setPiSim(string& str, size_t bitNum)
{
   if (str.length() == 0) 
      return false;
   str = str.substr(str.find_first_not_of(" "));
   if (str.length() != _piList.size()) {
      cerr << "Pattern(" << str << ") length(" << str.length() << ") does not match the number of inputs("
            << _piList.size() << ") in a circuit!!\n";
      return false;
   }
   for (size_t i = 0; i < _piList.size(); i++) {
      if (str[i] == '0') 
         _piList[i]->flipToZero(~(const1 << bitNum));
      
      else if (str[i] == '1') 
         _piList[i]->flipToOne(const1 << bitNum);
      else {
         cerr << "Pattern(" << str << ") contains a non-0/1 character('" << str[i] << "').\n";
         return false;
      }
   }
   return true;
}


void
CirMgr::randomSim()
{

}

void 
CirMgr::fraigSim(vector<string>& counterExample)
{
   size_t bitNum = 0, wordLen = patternLen;
   for (string& str : counterExample) {
      if (!setPiSim(str, bitNum))
         return;
      bitNum++;
      if (bitNum == wordLen) {
         simulate(bitNum);
         bitNum = 0;
      }
   }
   simulate(bitNum);
   if (_simLog) 
      _simLog->close();
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   string str;
   size_t bitNum = 0, wordLen = patternLen;
   while (getline(patternFile, str)) {
      if (!setPiSim(str, bitNum))
         return;
      bitNum++;
      if (bitNum == wordLen) {
         simulate(bitNum);
         bitNum = 0;
      }
   }

   simulate(bitNum);

   if (_simLog) 
      _simLog->close();
   
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
