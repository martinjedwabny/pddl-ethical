// ***********************************************************
// 
//  Book:       Heuristic Search
// 
//  Authors:    S.Edelkamp, S.Schroedl
// 
//  See file README for information on using and copying 
//  this software.
// 
//  Project:    Mips - model checking integrated planning
//              system
// 
//  Module:     mips\include\out.operator.h
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************

#ifndef _OPERATOR_
#define _OPERATOR_

class BitArray;
class FactMap;

#include <vector>
#include <list>
#include <bdd.h>

using namespace std;


class Operator {

  FactMap& factMap;

 public:

  bool derived;
  bool durative;
  int duration;
  double prune;

  double mintime;
  double maxtime;

  int mark;
  int number;
  int actionNumber;

  int pres;  int presize; int* pre; int* pre_t;
  int pdelsize; int* pdel;  int* pdel_t;
  int addsize; int* add;  int* add_t;
  int delsize; int* del; int* del_t;

  int lesize; int* le; int* le_v;
  int lsize; int* l; int* l_v;
  int gesize; int* ge; int* ge_v;
  int gsize; int* g; int* g_v;
  int esize; int* e; int* e_v;

  int asssize; int* ass; int* ass_v;
  int incsize; int* inc; int* inc_v;
  int decsize; int* dec; int* dec_v;

  int revsize;  int* rev;

  int whensize;
  int* cpresize; int** cpre; int** cpre_t;
  int* cpdelsize; int** cpdel; int** cpdel_t;
  int* caddsize; int** cadd; int** cadd_t;
  int* cdelsize; int** cdel; int** cdel_t;

  bdd currBdd;

  vector<pair<int,double> > preInt;
  vector<pair<int,double> > effInt;
  void generateIntLists(double** values,int* sizes);

  void swap();
  Operator(int nr, int derived, float duration, int durative,
	   vector<pair<int,int> >& Pre,
	   vector<pair<int,int> >& Pdel,
	   vector<pair<int,int> >& Add,
	   vector<pair<int,int> >& Del,
	   vector<pair<int,int> >& Numl,
	   vector<pair<int,int> >& Numle,
	   vector<pair<int,int> >& Numg,
	   vector<pair<int,int> >& Numge,
	   vector<pair<int,int> >& Nume,
	   vector<pair<int,int> >& Numass,
	   vector<pair<int,int> >& Numinc,
	   vector<pair<int,int> >& Numdec,
	   vector<vector<pair<int,int> > >& cPre,
	   vector<vector<pair<int,int> > >& cPdel,
	   vector<vector<pair<int,int> > >& cAdd,
	   vector<vector<pair<int,int> > >& cDel,
	   FactMap& fMap);

  Operator(const Operator& copy); 

  bool isDurative();
  void setNum(int i, double d);
  void setNum(int p1, double d1, int i, double d);
  void setNum(int p1, double d1, int p2, double d2, int i, double d);
  bool Noop();

  void buildBdd(bdd* frame);
  void buildBdd();
  bdd& getBdd() { return currBdd; }
  void buildBdd(bdd* frame,bool* isActive);

  void prepareUpdate();
  void prepareCheck();
  bool containsNegatedPrecondition(BitArray* vector);
  bool matchOneAdd(BitArray* vector);
  bool matchOnePred(BitArray* vector);
  bool matchOneDel(BitArray* vector);
  bool matchAllAdds(BitArray* vector);
  bool matchAllPreds(BitArray* vector);
  bool matchAllRevs(BitArray* vector);
  bool operator==(const Operator &other) const;
  bool operator<(const Operator &other) const;

  static bool equalfun(Operator* one,Operator* two)  {
    return *one==*two;
  }
  static bool comparefun(Operator* one,Operator* two)  {
    return *one<*two;
  }

  double getMin(int res, double* resource);
  double getMax(int res, double* resource);
  void apply(BitArray* bv);

  int getActionNr() { return actionNumber; }
  int getNr() { return number; }
  bool restrict(BitArray* vector);

  bool depend(Operator* other);
  bool endstart(Operator* other);
  bool endover(Operator* other);
  bool startover(Operator* other);
  bool startstart(Operator* other);
  bool endend(Operator* other);
  bool enable(Operator* other);

  void removePrecondition(int p);
	bool equals(Operator &other);
  void OpenTimeWindow(double time) { if (time > mintime) mintime = time; }
  void CloseTimeWindow(double time){ if (time < maxtime) maxtime = time; }

  string flushpre();
  string flushadd();
  string flushdel();
  string flushpdel();

  string flushdec();
  string flushass();
  string flushinc();

  string flushl();
  string flushle();
  string flushg();
  string flushge();
  string flushe();

  string flushwhen();
  string getString();
  string toString();
};

#endif
