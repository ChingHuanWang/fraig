/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <unordered_map>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

void
CirMgr::merge(CirGate* f, CirGate* g)
{
   vector<CirGateV>& foutList = g->getFoutList();
   for (CirGateV gatev : foutList) {
      CirGate* fout = gatev.getGate();
      if (fout->getIn0Gate() == g) 
         fout->setIn0((size_t)(f)+gatev.isInv());
      else
         fout->setIn1((size_t)(f)+gatev.isInv());
   }
}

bool
CirMgr::merge(CirGate* g)
{
   bool doMerge = false;
   if (g->isAig()) {
      HashKey key = HashKey(g->getIn0GateV(), g->getIn1GateV());
      unordered_map<HashKey, CirGate*>::const_iterator tmp = _gateHashTable.find(key);
      doMerge = (tmp == _gateHashTable.end());
      if (!doMerge) 
         _gateHashTable[key] = g;
      else 
         merge(tmp->second, g);
   }
   return doMerge;
}

void
CirMgr::strash()
{
   for (CirGate* g : _dfsList) {
      if (merge(g)) 
         recycle(g);     
   }
   reconstructCircuit();
}

void 
CirMgr::genProofModel(SatSolver& s)
{  
   for (CirGate* g : _idList) {
      if (g) {
         Var v = s.newVar();
         g->setVar(v);
      }
   }

   for (CirGate* g : _dfsList) {
      if (g->isAig()) 
         s.addAigCNF(g->getVar(), g->getIn0Gate()->getVar(), g->isIn0Inv(),
                                  g->getIn1Gate()->getVar(), g->isIn1Inv());
   }

   
   
}




bool
CirMgr::checkPair(SatSolver& s, size_t litId1, size_t litId2)
{
   Var fVar = s.newVar();
   s.addXorCNF(fVar, _idList[litId1/2]->getVar(), litId1%2 == 1,
                     _idList[litId2/2]->getVar(), litId2%2 == 1);
   s.assumeRelease();
   s.assumeProperty(fVar, true);
   s.assumeProperty(_const->getVar(), false);
   bool result = s.assumpSolve();
   return !result;
}

bool
CirMgr::checkGrp(SatSolver& s, FecGrp& grp, vector<string>& counterExample)
{
   FecGrp newGrp;
   bool isSame = false;
   size_t i = 0, j, litId1, litId2;
   newGrp.add(grp[i]);
   if (grp[i]/2 == 0) {
      i++;
      newGrp.add(grp[i]);
   }
   for (j = i+1; j < grp.size(); j++) {
      litId1 = grp[i];
      litId2 = grp[j];
      isSame = checkPair(s, litId1, litId2);
      if (isSame) {
         merge(_idList[litId1/2], _idList[litId2/2]);
         recycle(_idList[litId2/2]);
      } else {
         string example(_piList.size(), '0');
         for (size_t k = 0; k < _piList.size(); k++) 
            example[k] = s.getValue(_piList[k]->getVar()) == 1 ? '1' : '0';
            
         counterExample.push_back(example);
         newGrp.insert(grp.begin()+j, grp.end());
         break;
      }     
   }
   
   grp = newGrp;
   return isSame;
}

void
CirMgr::fraig()
{
   SatSolver s;
   s.initialize();
   genProofModel(s);
   vector<string> counterExample;
   for (FecGrp& grp : *_fecGrps) 
      checkGrp(s, grp, counterExample);
   
   reconstructCircuit();
   // cout << "counter example num = " << counterExample.size() << endl;
   fraigSim(counterExample);
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
