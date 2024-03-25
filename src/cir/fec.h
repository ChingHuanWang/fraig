
#ifndef CIR_FEC_H
#define CIR_FEC_H

#include <vector>
#include <bitset>
#include "cirDef.h"
#include <bits/stdc++.h>

class CirSimV
{
public:
   CirSimV(size_t v = 0) { _simV = v; }

   size_t operator()() const { return _simV; }
   size_t operator ~() const { return ~_simV; }
   CirSimV operator &(const CirSimV& p) const {
      CirSimV v = _simV & p._simV; return v; }
   CirSimV& operator &=(const CirSimV& p) {
      _simV &= p._simV; return *this; }
   CirSimV operator =(const CirSimV& p) {
      _simV = p._simV; return *this; }
   bool operator ==(const CirSimV& p) const {
      return (_simV == p._simV); }

   CirSimV& operator |=(const size_t& val) {
      _simV |= val; return *this;
   }

   CirSimV& operator &=(const size_t& val) {
      _simV &= val; return *this;
   }
   
   
private:
   size_t _simV;
};

class HashInt
{
public:
   size_t operator() (const size_t& k) const
   {
      return k ^ (k >> 32);
   }
};

class FecGrp
{
public:
   FecGrp() {}
   FecGrp(const vector<CirGate*>&);
   ~FecGrp() {}
   // size_t  operator[] (const size_t i) const { return _vec[i]; } 
   size_t operator[] (const size_t i) const { return _vec[i]; }
   size_t size() const { return _vec.size(); }
   vector<size_t>::iterator begin() { return _vec.begin(); }
   vector<size_t>::iterator end() { return _vec.end(); }
   void print() 
   {
      for (auto& e : _vec) cout << e << " ";
      cout << endl;
   }
   void sortGrp() { sort(_vec.begin(), _vec.end()); }
   void add(size_t litId) { _vec.push_back(litId); }
   void insert(vector<size_t>::iterator begin, vector<size_t>::iterator end) { _vec.insert(_vec.end(), begin, end); } 
   friend ostream& operator << (ostream& os, FecGrp& grp);
   FecGrp& operator = (FecGrp& k) 
   { 
      _vec.clear();
      _vec.insert(_vec.end(), k.begin(), k.end());
      return *this; 
   }

private:
   vector<size_t> _vec;
};

#endif // CIR_GATE_H