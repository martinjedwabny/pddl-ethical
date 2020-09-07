
#include <vector>

#include <util.options.h>
#include <util.tools.h>
#include <util.bitarray.h>

#include <out.factMap.h>
#include <out.operator.h>
// #include <out.group.h>

Operator::Operator(int nr, int der, float dur, int dura,
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
		   FactMap& fMap): 
  factMap(fMap), number(nr), derived(der),
  durative(dura), mark(0) {

  prune = false;
  mintime = 0;
  maxtime = 10000000;

  if (dur - ((int) dur) != 0) 
      cout << "WARNING: duration fractional, working on integer approximation" << endl;
////      ::error("duration fractional");
/*
      duration = (int) (dur * 100);
  else
*/ 
      duration = (int) dur;


  if (factMap.getActionName(number).substr(0,9) == "dont-care") {
      cout << "found dont care" << endl;
      mark = -1;
  }
//  else       
//      cout << "found pattern" << endl;

  pres = presize = Pre.size();
  pre = new int [presize];
  pre_t = new int [presize];
  for (int i=0;i<presize;i++) pre[i] = Pre[i].second;
  for (int i=0;i<presize;i++) pre_t[i] = Pre[i].first;
  pdelsize = Pdel.size();
  pdel = new int [pdelsize];
  pdel_t = new int [pdelsize];

  for (int i=0;i<pdelsize;i++) pdel[i] = Pdel[i].second;
  for (int i=0;i<pdelsize;i++) pdel_t[i] = Pdel[i].first;
  addsize = Add.size();
  add = new int [addsize];
  add_t = new int [addsize];
  for (int i=0;i<addsize;i++) add[i] = Add[i].second;
  for (int i=0;i<addsize;i++) add_t[i] = Add[i].first;
  delsize = Del.size();
  del = new int [delsize];
  del_t = new int [delsize];
  for (int i=0;i<delsize;i++) del[i] = Del[i].second; 
  for (int i=0;i<delsize;i++) del_t[i] = Del[i].first; 

  lsize = Numl.size();
  l = new int [lsize];
  l_v = new int [lsize];
  for (int i=0;i<lsize;i++) l[i] = Numl[i].second;
  for (int i=0;i<lsize;i++) l_v[i] = Numl[i].first;

  lesize = Numle.size();
  le = new int [lesize];
  le_v = new int [lesize];
  for (int i=0;i<lesize;i++) le[i] = Numle[i].second;
  for (int i=0;i<lesize;i++) le_v[i] = Numle[i].first;

  gsize = Numg.size();
  g = new int [gsize];
  g_v = new int [gsize];
  for (int i=0;i<gsize;i++) g[i] = Numg[i].second;
  for (int i=0;i<gsize;i++) g_v[i] = Numg[i].first;

  //  cout << "pd-size" << pdelsize << endl;

  gesize = Numge.size();
  ge = new int [gesize];
  ge_v = new int [gesize];
  for (int i=0;i<gesize;i++) ge[i] = Numge[i].second;
  for (int i=0;i<gesize;i++) ge_v[i] = Numge[i].first;

  esize = Nume.size();
  e = new int [esize];
  e_v = new int [esize];
  for (int i=0;i<esize;i++) e[i] = Nume[i].second;
  for (int i=0;i<esize;i++) e_v[i] = Nume[i].first;

  asssize = Numass.size();
  ass = new int [asssize];
  ass_v = new int [asssize];
  for (int i=0;i<asssize;i++) ass[i] = Numass[i].second;
  for (int i=0;i<asssize;i++) ass_v[i] = Numass[i].first;

  decsize = Numdec.size();
  dec = new int [decsize];
  dec_v = new int [decsize];
  for (int i=0;i<decsize;i++) dec[i] = Numdec[i].second;
  for (int i=0;i<decsize;i++) dec_v[i] = Numdec[i].first;

  incsize = Numinc.size();
  inc = new int [incsize];
  inc_v = new int [incsize];
  for (int i=0;i<incsize;i++) inc[i] = Numinc[i].second;
  for (int i=0;i<incsize;i++) inc_v[i] = Numinc[i].first;

  whensize = cPre.size();

  cpresize = new int[whensize]; 
  cpre = new int*[whensize]; 
  cpre_t = new int*[whensize];
  cpdelsize = new int[whensize]; 
  cpdel = new int*[whensize]; 
  cpdel_t = new int*[whensize]; 
  caddsize = new int[whensize]; 
  cadd = new int*[whensize]; 
  cadd_t = new int*[whensize]; 
  cdelsize = new int[whensize]; 
  cdel = new int*[whensize]; 
  cdel_t = new int*[whensize]; 

  for(int w=0;w<whensize;w++) {
      cpresize[w] = cPre[w].size();
      cpre[w] = new int [cpresize[w]];
      cpre_t[w] = new int [cpresize[w]];
      for (int i=0;i<cpresize[w];i++) 
	  cpre[w][i] = cPre[w][i].second;
      for (int i=0;i<cpresize[w];i++) 
	  cpre_t[w][i] = cPre[w][i].first;

      cpdelsize[w] = cPdel[w].size();
      cpdel[w] = new int [cpdelsize[w]];
      cpdel_t[w] = new int [cpdelsize[w]];
      for (int i=0;i<cpdelsize[w];i++) 
	  cpdel[w][i] = cPdel[w][i].second;
      for (int i=0;i<cpdelsize[w];i++) 
	  cpdel_t[w][i] = cPdel[w][i].first;

      caddsize[w] = cAdd[w].size();
      cadd[w] = new int [caddsize[w]];
      cadd_t[w] = new int [caddsize[w]];
      for (int i=0;i<caddsize[w];i++) 
	  cadd[w][i] = cAdd[w][i].second;
      for (int i=0;i<caddsize[w];i++) 
	  cadd_t[w][i] = cAdd[w][i].first;

      cdelsize[w] = cDel[w].size();
      cdel[w] = new int [cdelsize[w]];
      cdel_t[w] = new int [cdelsize[w]];

      for (int i=0;i<cdelsize[w];i++) 
	  cdel[w][i] = cDel[w][i].second; 
      for (int i=0;i<cdelsize[w];i++) 
	  cdel_t[w][i] = cDel[w][i].first; 
  }
 


  rev = new int [addsize+presize];
  revsize = 0;
  for(int i = 0; i < presize; i++) { 
    int j = 0; 
    while (j < delsize && del[j] != pre[i]) j++;
    if (j == delsize) // not in delete list
      rev[revsize++] = pre[i];
  }
  for(int i = 0; i < addsize; i++) 
    rev[revsize++] = add[i];

}

Operator::Operator(const Operator& copy): number(copy.number), 
                      factMap(copy.factMap) {

  whensize = copy.whensize;
  cpresize = new int[whensize]; 
  cpre = new int*[whensize]; 
  cpre_t = new int*[whensize];
  cpdelsize = new int[whensize]; 
  cpdel = new int*[whensize]; 
  cpdel_t = new int*[whensize]; 
  caddsize = new int[whensize]; 
  cadd = new int*[whensize]; 
  cadd_t = new int*[whensize]; 
  cdelsize = new int[whensize]; 
  cdel = new int*[whensize]; 
  cdel_t = new int*[whensize]; 

  for (int w=0;w<whensize;w++) {
      cpdelsize[w] = copy.cpdelsize[w];
      cpdel[w] = new int[cpdelsize[w]];  
      for(int i = 0; i < cpdelsize[w]; i++)  
	  cpdel[w][i] = copy.cpdel[w][i];
      
      cpresize[w] = copy.cpresize[w];
      cpre[w] = new int[cpresize[w]];  
      for(int i = 0; i < cpresize[w]; i++)  
	  cpre[w][i] = copy.cpre[w][i];
      
      caddsize[w] = copy.caddsize[w];
      cadd[w] = new int[caddsize[w]];  
      for(int i = 0; i < caddsize[w]; i++)  
	  cadd[w][i] = copy.cadd[w][i];
      
      cdelsize[w] = copy.cdelsize[w];
      cdel[w] = new int[cdelsize[w]];  
      for(int i = 0; i < cdelsize[w]; i++)  
	  cdel[w][i] = copy.cdel[w][i];
  }
  
  derived = copy.derived;
  duration = copy.duration;
  durative = copy.durative;
  mark = copy.mark;
  pres = presize = copy.presize;

  pdelsize = copy.pdelsize;
  pdel = new int[pdelsize];  
  for(int i = 0; i < pdelsize; i++)  
    pdel[i] = copy.pdel[i];

  pre = new int[presize];  
  for(int i = 0; i < presize; i++)  
    pre[i] = copy.pre[i];

  addsize = copy.addsize;
  add = new int[addsize];  
  for(int i = 0; i < addsize; i++)  
    add[i] = copy.add[i];

  delsize = copy.delsize;
  del = new int[delsize];  
  for(int i = 0; i < delsize; i++)  
    del[i] = copy.del[i];

  revsize = copy.revsize;
  rev = new int[revsize];  
  for(int i = 0; i < revsize; i++)  
    rev[i] = copy.rev[i];
}


void Operator::buildBdd(bdd* frame) {
  bdd preBdd = bddtrue;
  // cout << " --- combining sub-BDDs " << endl;
  for(int i = 0; i < presize; i++) {
    int fact = pre[i];
    // cout << "pre " << factMap.getFactName(fact) << endl;  
    bdd preon = *factMap.getPreAddFactBdd(fact);
    // bdd_printtable(preon);
    preBdd &= preon;
    // frame[factMap.getGroup(fact)] = bddtrue;
  }
  bdd npreBdd = bddtrue;
  for(int i = 0; i < pdelsize; i++) {
    int fact = pdel[i];
    // cout << "npre " << factMap.getFactName(fact) << endl;  
    bdd npreon = *factMap.getPreDelFactBdd(fact);
    // bdd_printtable(npreon);
    npreBdd &= npreon;
     frame[factMap.getGroup(fact)] = bddtrue;
  }

  bdd delBdd = bddtrue;
  for(int i = 0; i < delsize; i++) {
    int fact = del[i];
    // cout << "del " << factMap.getFactName(fact) << endl;  
    int j;
    for(j = 0; j < addsize; j++) { 
      if (factMap.getGroup(fact) == factMap.getGroup(add[j]))
	break;
    }
    if (j != addsize) continue;
    bdd delon = *factMap.getEffDelFactBdd(fact);
    //    bdd_printtable(delon);
    delBdd &= delon;
    frame[factMap.getGroup(fact)] = bddtrue;
  }
  bdd addBdd = bddtrue;
  for(int i = 0; i < addsize; i++) {
    int fact = add[i];
    // cout << "eff " << factMap.getFactName(fact) << endl;  
    bdd addon = *factMap.getEffAddFactBdd(fact);
    // bdd_printtable(addon);
    addBdd &= addon;
    frame[factMap.getGroup(fact)] = bddtrue;
  }

    bdd lBdd = bddtrue;
    for(int i = 0; i < lsize; i++) {
	int fact = l[i];
	//cout << "num-fact < " << factMap.getFactName(fact) << endl;  
	int value = l_v[i];
	bdd lon = *factMap.getPreLessFactBdd(fact,value);
	lBdd &= lon;
    }

    bdd leBdd = bddtrue;
    for(int i = 0; i < lesize; i++) {
	int fact = le[i];
	int value = le_v[i];
	bdd leon = *factMap.getPreLessEqualFactBdd(fact,value);
	leBdd &= leon;
    }

    bdd gBdd = bddtrue;
    for(int i = 0; i < gsize; i++) {
	int fact = g[i];
	int value = g_v[i];
	bdd gon = *factMap.getPreGreaterFactBdd(fact,value);
	gBdd &= gon;
    }

    bdd geBdd = bddtrue;
    for(int i = 0; i < gesize; i++) {
	int fact = ge[i];
	int value = ge_v[i];
	bdd geon = *factMap.getPreGreaterEqualFactBdd(fact,value);
	//bdd_printtable(geon);
	geBdd &= geon;
    }

    bdd eBdd = bddtrue;
    for(int i = 0; i < gsize; i++) {
	int fact = e[i];
	int value = e_v[i];
	bdd eon = *factMap.getPreEqualFactBdd(fact,value);
	eBdd &= eon;
    }

    bdd incBdd = bddtrue;
    for(int i = 0; i < incsize; i++) {
	int fact = inc[i];
	int value = inc_v[i];
	// cout << "inc eff " << factMap.getFactName(fact) << endl;  
	bdd incon = *factMap.getEffIncFactBdd(fact,value);
	// bdd_printtable(incon);
        incBdd &= incon;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    bdd decBdd = bddtrue;
    for(int i = 0; i < decsize; i++) {
	int fact = dec[i];
	int value = dec_v[i];
//        cout << "dec eff " << factMap.getFactName(fact) << endl;  
	bdd decon = *factMap.getEffDecFactBdd(fact,value);
	// bdd_printtable(decon);
        decBdd &= decon;
//	cout << "group" << factMap.getGroup(fact) << endl;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    bdd assBdd = bddtrue;
    for(int i = 0; i < asssize; i++) {
	int fact = ass[i];
	int value = ass_v[i];
	// cout << "ass eff " << factMap.getFactName(fact) << endl;  
	bdd asson = *factMap.getEffAssFactBdd(fact,value);
	// bdd_printtable(asson);
        assBdd &= asson;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    currBdd = 
	npreBdd & preBdd & addBdd & delBdd &
	lBdd & leBdd & gBdd & geBdd & eBdd &
	assBdd & incBdd & decBdd;

//  currBdd = npreBdd & preBdd & addBdd & delBdd;

  // bdd_printtable(currBdd);

  //  cout << " --- done"  << bdd_nodecount(currBdd) << endl;
  //  cout << " --- building frame "  << endl;
  bdd tempBdd = bddtrue;

  for(int i = 0; i < factMap.groupFullCount(); i++)
    if (frame[i] == bddfalse)
      tempBdd &= factMap.getFdd(i);

  //  cout << " --- done: " << bdd_nodecount(tempBdd) << " merging frame " << endl;
  currBdd &= tempBdd;
  //  cout << " --- done " << bdd_nodecount(currBdd) << endl;


  if (whensize > 0) {
      currBdd = bddfalse;
      bdd whenBdd = npreBdd & preBdd & addBdd & delBdd ;
      for (int w=0;w<whensize;w++) {
	  bdd voidbdd = bddtrue;
	  
	  bdd cpreBdd = bddtrue;
	  bdd cnpreBdd = bddtrue;
	  if (cpresize[w] > 1) {
	      ::error("complex conditional precondition");
	  }
	  for(int i = 0; i < cpresize[w]; i++) {
	      int fact = cpre[w][i];
	      // cout << "pre " << factMap.getFactName(fact) << endl;  
	      bdd cpreon = *factMap.getPreAddFactBdd(fact);
	      bdd cnpreon = *factMap.getPreDelFactBdd(fact);
	      cnpreBdd &= cnpreon;
	      // bdd_printtable(cpreon);
	      cpreBdd &= cpreon;
//	  frame[factMap.getGroup(fact)] = bddtrue;
	  }
	  if (cpdelsize[w] > 0) {
	      ::error("negated conditional precondition");
	  }
	  for(int i = 0; i < cpdelsize[w]; i++) {
	  int fact = cpdel[w][i];
	  bdd cnpreon = *factMap.getPreDelFactBdd(fact);
	  cnpreBdd &= cnpreon;
//	  frame[factMap.getGroup(fact)] = bddtrue;
	  }
	  bdd cdelBdd = bddtrue;
	  for(int i = 0; i < cdelsize[w]; i++) {
	      int fact = cdel[w][i];
//	      cout << "cadd " << factMap.getFactName(fact) << endl;  
	      int j;
	      for(j = 0; j < caddsize[w]; j++) { 
		  if (factMap.getGroup(fact) == factMap.getGroup(cadd[w][j]))
		      break;
	      }
	      if (j != caddsize[w]) continue;
	      bdd cdelon = *factMap.getEffDelFactBdd(fact);
	      cdelBdd &= cdelon;
	      frame[factMap.getGroup(fact)] = bddtrue;
	      voidbdd &= factMap.getFdd(factMap.getGroup(fact));
	  }
	  bdd caddBdd = bddtrue;
	  for(int i = 0; i < caddsize[w]; i++) {
	      int fact = cadd[w][i];
//	   cout << "ceff " << factMap.getFactName(fact) << endl;  
	      bdd caddon = *factMap.getEffAddFactBdd(fact);
	      caddBdd &= caddon;
	      frame[factMap.getGroup(fact)] = bddtrue;
	      voidbdd &= factMap.getFdd(factMap.getGroup(fact));
	  }
//      bdd_printtable(voidbdd);
	  
	  
	  whenBdd &= 
	      ((cpreBdd) & (caddBdd & cdelBdd));
      }
      tempBdd = bddtrue;
      for(int i = 0; i < factMap.groupFullCount(); i++)
	  if (frame[i] == bddfalse)
	      tempBdd &= factMap.getFdd(i);
      
  //  cout << " --- done: " << bdd_nodecount(tempBdd) << " merging frame " << endl;
      
      whenBdd &= tempBdd;
      
      currBdd = whenBdd;
  }
//      whenbdd &= 
//	  ((cpreBdd) & (caddBdd & cdelBdd)) |
//	  (!(cpreBdd) & (voidbdd));


// | 
//	  ((!(cpreBdd & cnpreBdd)) & preBdd & npreBdd & voidbdd);  
//      whenbdd &= ((cpreBdd & cnpreBdd) >> (caddBdd & cdelBdd));
//      | voidbdd; // (!(cpreBdd & cnpreBdd) & voidbdd);  exit(1);
      // conditional-preconditions => conditionals-effects
      // no conditional-preconditions => no effects
 

  if (options.strategy() == Options::WEAK ||
      options.strategy() == Options::STRONG ||
      options.strategy() == Options::STRONGCYCLIC)
    currBdd &= fdd_ithvar(factMap.getActIndex(),number);
  // bdd_printtable(currBdd);
  //exit(1);
}


void Operator::buildBdd() {
  bdd preBdd = bddtrue;
  for(int i = 0; i < presize; i++) {
    int fact = pre[i];
    // cout << "pre " << factMap.getFactName(fact) << endl;  
    bdd preon = *factMap.getPreAddFactBdd(fact);
    // bdd_printtable(preon);
    preBdd &= preon;
  }
   bdd npreBdd = bddtrue;
  for(int i = 0; i < pdelsize; i++) {
    int fact = pdel[i];
    bdd npreon = *factMap.getPreDelFactBdd(fact);
    npreBdd &= npreon;
  }
  bdd delBdd = bddtrue;
  for(int i = 0; i < delsize; i++) {
    int fact = del[i];
    // cout << "del " << factMap.getFactName(fact) << endl;  
    int j;
    for(j = 0; j < addsize; j++) { 
      if (factMap.getGroup(fact) == 
      factMap.getGroup(add[j]))
    break;
    }
    if (j != addsize) continue;
    bdd delon = *factMap.getEffDelFactBdd(fact);
    //    bdd_printtable(delon);
    delBdd &= delon;
  }
  bdd addBdd = bddtrue;
  for(int i = 0; i < addsize; i++) {
    int fact = add[i];
    // cout << "eff " << factMap.getFactName(fact) << endl;  
    bdd addon = *factMap.getEffAddFactBdd(fact);
    // bdd_printtable(addon);
    addBdd &= addon;
  }

    bdd lBdd = bddtrue;
    for(int i = 0; i < lsize; i++) {
	int fact = l[i];
	int value = l_v[i];
	bdd lon = *factMap.getPreLessFactBdd(fact,value);
	lBdd &= lon;
    }

    bdd leBdd = bddtrue;
    for(int i = 0; i < lesize; i++) {
	int fact = le[i];
	int value = le_v[i];
	bdd leon = *factMap.getPreLessEqualFactBdd(fact,value);
	leBdd &= leon;
    }

    bdd gBdd = bddtrue;
    for(int i = 0; i < gsize; i++) {
	int fact = g[i];
	int value = g_v[i];
	bdd gon = *factMap.getPreGreaterFactBdd(fact,value);
	gBdd &= gon;
    }

    bdd geBdd = bddtrue;
    for(int i = 0; i < gesize; i++) {
	int fact = ge[i];
	int value = ge_v[i];
	bdd geon = *factMap.getPreGreaterEqualFactBdd(fact,value);
	bdd_printtable(geon);
	geBdd &= geon;
    }

    bdd eBdd = bddtrue;
    for(int i = 0; i < gsize; i++) {
	int fact = e[i];
	int value = e_v[i];
	bdd eon = *factMap.getPreEqualFactBdd(fact,value);
	eBdd &= eon;
    }

    bdd incBdd = bddtrue;
    for(int i = 0; i < incsize; i++) {
	int fact = inc[i];
	int value = inc_v[i];
	// cout << "inc eff " << factMap.getFactName(fact) << endl;  
	bdd incon = *factMap.getEffIncFactBdd(fact,value);
	// bdd_printtable(incon);
        incBdd &= incon;
    }

    bdd decBdd = bddtrue;
    for(int i = 0; i < decsize; i++) {
	int fact = dec[i];
	int value = dec_v[i];
	// cout << "dec eff " << factMap.getFactName(fact) << endl;  
	bdd decon = *factMap.getEffDecFactBdd(fact,value);
	// bdd_printtable(decon);
        decBdd &= decon;
    }

    bdd assBdd = bddtrue;
    for(int i = 0; i < asssize; i++) {
	int fact = ass[i];
	int value = ass_v[i];
	// cout << "ass eff " << factMap.getFactName(fact) << endl;  
	bdd asson = *factMap.getEffAssFactBdd(fact,value);
	// bdd_printtable(asson);
        assBdd &= asson;
    }
    


  bdd whenbdd = bddtrue;
  for (int w=0;w<whensize;w++) {
      bdd cpreBdd = bddtrue;
      for(int i = 0; i < cpresize[w]; i++) {
	  int fact = cpre[w][i];
	  // cout << "pre " << factMap.getFactName(fact) << endl;  
	  bdd cpreon = *factMap.getPreAddFactBdd(fact);
	  // bdd_printtable(cpreon);
	  cpreBdd &= cpreon;
      }
      bdd cnpreBdd = bddtrue;
      for(int i = 0; i < cpdelsize[w]; i++) {
	  int fact = cpdel[w][i];
	  bdd cnpreon = *factMap.getPreDelFactBdd(fact);
	  cnpreBdd &= cnpreon;
      }
      bdd cdelBdd = bddtrue;
      for(int i = 0; i < cdelsize[w]; i++) {
	  int fact = cdel[w][i];
    // cout << "del " << factMap.getFactName(fact) << endl;  
	  int j;
	  for(j = 0; j < caddsize[w]; j++) { 
	      if (factMap.getGroup(fact) == 
		  factMap.getGroup(cadd[w][j]))
		  break;
	  }
	  if (j != caddsize[w]) continue;
	  bdd cdelon = *factMap.getEffDelFactBdd(fact);
	  //    bdd_printtable(cdelon);
	  cdelBdd &= cdelon;
      }
      bdd caddBdd = bddtrue;
      for(int i = 0; i < caddsize[w]; i++) {
	  int fact = cadd[w][i];
	  // cout << "ceff " << factMap.getFactName(fact) << endl;  
	  bdd caddon = *factMap.getEffAddFactBdd(fact);
	  // bdd_printtable(addon);
	  caddBdd &= caddon;
      }
      whenbdd &= (cpreBdd & cnpreBdd) & (caddBdd & cdelBdd);
  }

    currBdd = 
	npreBdd & preBdd & addBdd & delBdd &
	lBdd & leBdd & gBdd & geBdd & eBdd &
	assBdd & incBdd & decBdd;

//  currBdd = npreBdd & preBdd & addBdd & delBdd & whenbdd;
  // bdd_printtable(currBdd);

if (options.strategy() == Options::WEAK ||
  options.strategy() == Options::STRONG ||
      options.strategy() == Options::STRONGCYCLIC)
      currBdd &= fdd_ithvar(factMap.getActIndex(),number);
// bdd_printtable(currBdd);
  //exit(1);
}


void Operator::buildBdd(bdd* frame,bool* isActive) {
    bdd preBdd = bddtrue;
    for(int i = 0; i < presize; i++) {
	int fact = pre[i];
	bdd preon = *factMap.getPreAddFactBdd(fact);
	preBdd &= preon;
    }
    bdd npreBdd = bddtrue;
    for(int i = 0; i < pdelsize; i++) {
	int fact = pdel[i];
	bdd npreon = *factMap.getPreDelFactBdd(fact);
	npreBdd &= npreon;
    }


    bdd delBdd = bddtrue;
    for(int i = 0; i < delsize; i++) {
	int fact = del[i];
	int j;
	for(j = 0; j < addsize; j++) { 
	    if (factMap.getGroup(fact) == 
		factMap.getGroup(add[j]))
		break;
	}
	if (j != addsize) continue;
	bdd delon = *factMap.getEffDelFactBdd(fact);
	//    bdd_printtable(delon);
	delBdd &= delon;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    bdd addBdd = bddtrue;
    for(int i = 0; i < addsize; i++) {
	int fact = add[i];
	// cout << "eff " << factMap.getFactName(fact) << endl;  
	bdd addon = *factMap.getEffAddFactBdd(fact);
	// bdd_printtable(addon);
	addBdd &= addon;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    bdd lBdd = bddtrue;
    for(int i = 0; i < lsize; i++) {
	int fact = l[i];
	int value = l_v[i];
	bdd lon = *factMap.getPreLessFactBdd(fact,value);
	lBdd &= lon;
    }

    bdd leBdd = bddtrue;
    for(int i = 0; i < lesize; i++) {
	int fact = le[i];
	int value = le_v[i];
	bdd leon = *factMap.getPreLessEqualFactBdd(fact,value);
	leBdd &= leon;
    }

    bdd gBdd = bddtrue;
    for(int i = 0; i < gsize; i++) {
	int fact = g[i];
	int value = g_v[i];
	bdd gon = *factMap.getPreGreaterFactBdd(fact,value);
	gBdd &= gon;
    }

    bdd geBdd = bddtrue;
    for(int i = 0; i < gesize; i++) {
	int fact = ge[i];
	int value = ge_v[i];
	bdd geon = *factMap.getPreGreaterEqualFactBdd(fact,value);
	bdd_printtable(geon);
	geBdd &= geon;
    }

    bdd eBdd = bddtrue;
    for(int i = 0; i < gsize; i++) {
	int fact = e[i];
	int value = e_v[i];
	bdd eon = *factMap.getPreEqualFactBdd(fact,value);
	eBdd &= eon;
    }

    bdd incBdd = bddtrue;
    for(int i = 0; i < incsize; i++) {
	int fact = inc[i];
	int value = inc_v[i];
	// cout << "inc eff " << factMap.getFactName(fact) << endl;  
	bdd incon = *factMap.getEffIncFactBdd(fact,value);
	// bdd_printtable(incon);
        incBdd &= incon;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    bdd decBdd = bddtrue;
    for(int i = 0; i < decsize; i++) {
	int fact = dec[i];
	int value = dec_v[i];
	// cout << "dec eff " << factMap.getFactName(fact) << endl;  
	bdd decon = *factMap.getEffDecFactBdd(fact,value);
	// bdd_printtable(decon);
        decBdd &= decon;
	frame[factMap.getGroup(fact)] = bddtrue;
    }

    bdd assBdd = bddtrue;
    for(int i = 0; i < asssize; i++) {
	int fact = ass[i];
	int value = ass_v[i];
	// cout << "ass eff " << factMap.getFactName(fact) << endl;  
	bdd asson = *factMap.getEffAssFactBdd(fact,value);
	// bdd_printtable(asson);
        assBdd &= asson;
	frame[factMap.getGroup(fact)] = bddtrue;
    }
    
    currBdd = 
	npreBdd & preBdd & addBdd & delBdd &
	lBdd & leBdd & gBdd & geBdd & eBdd &
	assBdd & incBdd & decBdd;
    
    bdd whenBdd = currBdd;
    
    for(int i = 0; i < factMap.groupCount(); i++)
	if (isActive[i] && frame[i] == bddfalse)
	    currBdd &= factMap.getFdd(i);


  if (whensize > 0) {
      currBdd = bddfalse;
      for (int w=0;w<whensize;w++) {
	  bdd cpreBdd = bddtrue;
	  for(int i = 0; i < cpresize[w]; i++) {
	      int fact = cpre[w][i];
	      // cout << "pre " << factMap.getFactName(fact) << endl;  
	      bdd cpreon = *factMap.getPreAddFactBdd(fact);
	      // bdd_printtable(preon);
	      cpreBdd &= cpreon;
	      // frame[factMap.getGroup(fact)] = bddtrue;
	  }
	  bdd cnpreBdd = bddtrue;
	  for(int i = 0; i < cpdelsize[w]; i++) {
	      int fact = cpdel[w][i];
	      bdd cnpreon = *factMap.getPreDelFactBdd(fact);
	      cnpreBdd &= cnpreon;
	      // frame[factMap.getGroup(fact)] = bddtrue;
	  }
	  bdd cdelBdd = bddtrue;
	  for(int i = 0; i < cdelsize[w]; i++) {
	      int fact = cdel[w][i];
	      // cout << "del " << factMap.getFactName(fact) << endl;  
	      int j;
	      for(j = 0; j < caddsize[w]; j++) { 
		  if (factMap.getGroup(fact) == 
		      factMap.getGroup(cadd[w][j]))
		      break;
	      }
	      if (j != caddsize[w]) continue;
	      bdd cdelon = *factMap.getEffDelFactBdd(fact);
	      //    bdd_printtable(cdelon);
	      cdelBdd &= cdelon;
	      frame[factMap.getGroup(fact)] = bddtrue;
	  }
	  bdd caddBdd = bddtrue;
	  for(int i = 0; i < caddsize[w]; i++) {
	      int fact = cadd[w][i];
	      // cout << "ceff " << factMap.getFactName(fact) << endl;  
	      bdd caddon = *factMap.getEffAddFactBdd(fact);
	      // bdd_printtable(addon);
	      caddBdd &= caddon;
	      frame[factMap.getGroup(fact)] = bddtrue;
	  }
	  whenBdd &= ((cpreBdd) & (caddBdd & cdelBdd));
      }
      bdd tempBdd = bddtrue;

      for(int i = 0; i < factMap.groupFullCount(); i++)
	  if (isActive[i] && frame[i] == bddfalse)
	      tempBdd &= factMap.getFdd(i);
      
      //  cout << " --- done: " << bdd_nodecount(tempBdd) << " merging frame " << endl;
      
      whenBdd &= tempBdd;
      
      currBdd = whenBdd;

  }
  

  // bdd_printtable(currBdd);


}

void Operator::swap() {
  int* temp = pre;
  pre = add;
  add = temp;
  int t = presize;
  presize = addsize;
  addsize = t;
  pres = addsize;
} 

void Operator::apply(BitArray* bv) {
  for (int j=0; j< delsize;j++)
    bv->clear(del[j]);
  for (int j=0; j< addsize;j++)
    bv->set(add[j]);
/*
  for (int ops = 0; ops < whensize; ops++) {
    if (whenClauses[ops]->matchAllPreds(bv)) {
      if (whenClauses[ops]->containsNegatedPrecondition(bv))
	continue;
      for (int j=0; j< whenClauses[ops]->delsize;j++)
	bv->clear(whenClauses[ops]->del[j]);
      for (int j=0; j< whenClauses[ops]->addsize;j++)
	bv->set(whenClauses[ops]->add[j]);
    }
  }
*/
}

void Operator::removePrecondition(int p) {
  presize--;
  for(int i=p; i < presize; i++) {
    pre[i] = pre[i+1];
    pre_t[i] = pre_t[i+1];
  }
}


bool Operator::equals(Operator &other) {
  if (addsize != other.addsize) return false;
  if (delsize != other.delsize) return false;
  if (presize != other.presize) return false;
  if (pdelsize != other.pdelsize) return false;
  if (gsize != other.gsize) return false;
  if (gesize != other.gesize) return false;
  if (lsize != other.lsize) return false;
  if (lesize != other.lesize) return false;
  if (esize != other.esize) return false;
  if (incsize != other.incsize) return false;
  if (decsize != other.decsize) return false;

  for(int i = 0; i < addsize; i++)  
    if (add[i] != other.add[i]) return false;
  for(int i = 0; i < delsize; i++)  
    if (del[i] != other.del[i]) return false;
  for(int i = 0; i < presize; i++)    
    if (pre[i] != other.pre[i]) return false;
  for(int i = 0; i < pdelsize; i++)    
    if (pdel[i] != other.pdel[i]) return false; 
  for(int i = 0; i < lesize; i++)    
    if (le[i] != other.le[i]) return false; 
  for(int i = 0; i < lesize; i++)    
    if (l[i] != other.l[i]) return false; 
  for(int i = 0; i <gsize; i++)    
    if (g[i] != other.g[i]) return false; 
  for(int i = 0; i < lesize; i++)    
    if (ge[i] != other.ge[i]) return false; 
  for(int i = 0; i < esize; i++)    
    if (e[i] != other.e[i]) return false; 
	for(int i = 0; i < lesize; i++)    
    if (le_v[i] != other.le_v[i]) return false; 
  for(int i = 0; i < lesize; i++)    
    if (l_v[i] != other.l_v[i]) return false; 
  for(int i = 0; i <gsize; i++)    
    if (g_v[i] != other.g_v[i]) return false; 
  for(int i = 0; i < gesize; i++)    
    if (ge_v[i] != other.ge_v[i]) return false; 
  for(int i = 0; i < esize; i++)    
    if (e_v[i] != other.e_v[i]) return false;
  for (int i = 0; i < incsize; i++) {
    if (inc[i] != other.inc[i]) return false;
    if (inc_v[i] != other.inc_v[i]) return false;
  }
  for (int i = 0; i < decsize; i++) {
    if (dec[i] != other.dec[i]) return false;
    if (dec_v[i] != other.dec_v[i]) return false;
  }

  return true;
}


bool Operator::enable(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) return true;
  return false;
}


bool Operator::startover(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) 
	if (add_t[i] == 1 && other->pre_t[j] == 2)
	  return true;
  
  for(int i = 0; i < delsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (del[i] == other->pre[j]) 
	if (del_t[i] == 1 && other->pre_t[j] == 2)
	  return true;


/*
  for (int ops2 = 0; ops2 < other->whensize; ops2++) {
    if (startover(other->whenClauses[ops2]))
      return true;
  }
  for (int ops1 = 0; ops1 < whensize; ops1++) {
    if (whenClauses[ops1]->startover(other))
      return true;
    for (int ops2 = 0; ops2 < other->whensize; ops2++) {
      if (whenClauses[ops1]->startover(other->whenClauses[ops2])) 
	return true;
    }
  }
*/
  return false;
}


bool Operator::startstart(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) 
	if (add_t[i] == 1 && other->pre_t[j] == 1)
	  return true;
  
  for(int i = 0; i < delsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (del[i] == other->pre[j]) 
	if (del_t[i] == 1 && other->pre_t[j] == 1)
	  return true;

  for(int i = 0; i < presize; i++) {
    for(int j = 0; j < other->addsize; j++)  
      if (pre[i] == other->add[j]) 
	if (pre_t[i] == 1 && other->add_t[j] == 1)
	  return true;
    
    for(int j = 0; j < other->delsize; j++)  
      if (pre[i] == other->del[j]) 
	if (pre_t[i] == 1 && other->del_t[j] == 1)
	  return true;
  }
  
  /*
  for (int ops2 = 0; ops2 < other->whensize; ops2++) {
    if (startstart(other->whenClauses[ops2]))
      return true;
  }
  for (int ops1 = 0; ops1 < whensize; ops1++) {
    if (whenClauses[ops1]->startstart(other))
      return true;
    for (int ops2 = 0; ops2 < other->whensize; ops2++) {
      if (whenClauses[ops1]->startstart(other->whenClauses[ops2])) 
	return true;
    }
  }
  */
  return false;
}



bool Operator::endend(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) 
	if (add_t[i] == 3 && other->pre_t[j] == 3)
	  return true;
  
  for(int i = 0; i < delsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (del[i] == other->pre[j]) 
	if (del_t[i] == 3 && other->pre_t[j] == 3)
	  return true;


  for(int i = 0; i < presize; i++) {
    for(int j = 0; j < other->addsize; j++)  
      if (pre[i] == other->add[j]) 
	if (pre_t[i] == 3 && other->add_t[j] == 3)
	  return true;
    
    for(int j = 0; j < other->delsize; j++)  
      if (pre[i] == other->del[j]) 
	if (pre_t[i] == 3 && other->del_t[j] == 3)
	  return true;
  }

/*
  for (int ops2 = 0; ops2 < other->whensize; ops2++) {
    if (endend(other->whenClauses[ops2]))
      return true;
  }
  for (int ops1 = 0; ops1 < whensize; ops1++) {
    if (whenClauses[ops1]->endend(other))
      return true;
    for (int ops2 = 0; ops2 < other->whensize; ops2++) {
      if (whenClauses[ops1]->endend(other->whenClauses[ops2])) 
	return true;
    }
  }
*/
  return false;
}



bool Operator::endstart(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) 
	if (add_t[i] == 3 && other->pre_t[j] == 1)
	  return true;
  
  for(int i = 0; i < delsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (del[i] == other->pre[j]) 
	if (del_t[i] == 3 && other->pre_t[j] == 1)
	  return true;

/*
  for (int ops2 = 0; ops2 < other->whensize; ops2++) {
    if (endstart(other->whenClauses[ops2]))
      return true;
  }
  for (int ops1 = 0; ops1 < whensize; ops1++) {
    if (whenClauses[ops1]->endstart(other))
      return true;
    for (int ops2 = 0; ops2 < other->whensize; ops2++) {
      if (whenClauses[ops1]->endstart(other->whenClauses[ops2])) 
	return true;
    }
  }
*/
  return false;
}

bool Operator::endover(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) 
	if (add_t[i] == 3 && other->pre_t[j] == 2) 
	  return true;
  
  for(int i = 0; i < delsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (del[i] == other->pre[j]) 
	if (del_t[i] == 3 && other->pre_t[j] == 2)
	  return true;
  /*
  for (int ops2 = 0; ops2 < other->whensize; ops2++) 
    if (endover(other->whenClauses[ops2]))
      return true;

  for (int ops1 = 0; ops1 < whensize; ops1++) {
    if (whenClauses[ops1]->endover(other))
      return true;
    for (int ops2 = 0; ops2 < other->whensize; ops2++) {
      if (whenClauses[ops1]->endover(other->whenClauses[ops2])) 
	return true;
    }
  }
  */
  return false;
}

bool Operator::depend(Operator* other) {
  for(int i = 0; i < addsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (add[i] == other->pre[j]) return true;
  for(int i = 0; i < delsize; i++)  
    for(int j = 0; j < other->presize; j++)  
      if (del[i] == other->pre[j]) return true;
  for(int i = 0; i < presize; i++) {
    for(int j = 0; j < other->addsize; j++)  
      if (pre[i] == other->add[j]) return true;
    for(int j = 0; j < other->delsize; j++)  
      if (pre[i] == other->del[j]) return true;
  }
/*
  for (int ops2 = 0; ops2 < other->whensize; ops2++) 
    if (depend(other->whenClauses[ops2]))
      return true;
  for (int ops1 = 0; ops1 < whensize; ops1++) {
    if (whenClauses[ops1]->depend(other))
      return true;
    for (int ops2 = 0; ops2 < other->whensize; ops2++) 
      if (whenClauses[ops1]->depend(other->whenClauses[ops2])) 
	return true;
  }
*/
  return false;
}
 
bool Operator::Noop() {
  bool exists, forall;
  forall = true;
  for(int i = 0;i<presize;i++) {
    exists = false;
    for(int j = 0;j<addsize;j++)
      exists |= (pre[i] == add[j]);
    forall &= exists;
  }
  return forall;
}

bool Operator::containsNegatedPrecondition(BitArray* vector) {
  for(int i = 0;i<pdelsize;i++)
    if (vector->get(pdel[i])) return true;
  return false;
}

bool Operator::matchOneAdd(BitArray* vector) {
  for(int i = 0;i<addsize;i++)
    if (vector->get(add[i])) return true;
  return false;
}

bool Operator::matchOneDel(BitArray* vector) {
  for(int i = 0;i<delsize;i++)
    if (vector->get(del[i])) return true;
  return false;
}

bool Operator::matchOnePred(BitArray* vector) {
  for(int i = 0;i<presize;i++)
    if (vector->get(pre[i])) return true;
  return false;
}

bool Operator::matchAllAdds(BitArray* vector) {
  for(int i = 0;i<addsize;i++)
    if (!vector->get(add[i])) return false;
  return true;
}

bool Operator::matchAllPreds(BitArray* vector) {
  for(int i = 0;i<presize;i++)
    if (!vector->get(pre[i])) return false;
  return true;  
}

bool Operator::matchAllRevs(BitArray* vector) {
  for(int i = 0;i<revsize;i++) 
    if (!vector->get(rev[i]))  return false;
  return true;  
}
    
string Operator::flushpre() {
  string back;
  for(int i = 0; i < presize; i++) {
    back += "      ";
    if (pre_t[i] == 1) back += "(at start ";
    if (pre_t[i] == 2) back += "(over all ";
    if (pre_t[i] == 3) back += "(at end ";
    back +=  "(" + factMap.getFactName(pre[i])+ ") ";
    if (pre_t[i]) back += ")";
    back += "\n";
  }
  return back;
}

string Operator::flushadd() {
  string back;
  for(int i = 0; i < addsize; i++) {
    back += "      ";
    if (add_t[i] == 1) back += "(at start ";
    if (add_t[i] == 2) back += "(over all ";
    if (add_t[i] == 3) back += "(at end ";
    back +=  "(" + factMap.getFactName(add[i]) + ") ";
    if (add_t[i]) back += ")";
    back += "\n";
  }
  return back;
}
string Operator::flushdel() {
  string back;
  for(int i = 0; i < delsize; i++) {
    back += "      ";
    if (del_t[i] == 1) back += "(at start ";
    if (del_t[i] == 2) back += "(over all ";
    if (del_t[i] == 3) back += "(at end ";
    back +=  "(not (" + factMap.getFactName(del[i]) + ")) ";
    if (del_t[i]) back += ")";
    back += "\n";
  }
  return back;
}

string Operator::flushpdel() {
  string back;
  for(int i = 0; i < pdelsize; i++) {
    back += "      ";
    if (pdel_t[i] == 1) back += "(at start ";
    if (pdel_t[i] == 2) back += "(over all ";
    if (pdel_t[i] == 3) back += "(at end ";
    back +=  " (not (" + factMap.getFactName(pdel[i]) + ")) ";
    if (pdel_t[i]) back += ")";
    back += "\n";
  }
  return back;
}



string Operator::flushl() {
  string back;
  for(int i = 0; i < lsize; i++) {
    back += "      ";
    back +=  "(< (" + factMap.getFactName(l[i]) + ") ";
    back += ::toString(l_v[i]) + ")";
    back += "\n";
  }
  return back;
}

string Operator::flushle() {
  string back;
  for(int i = 0; i < lesize; i++) {
    back += "      ";
    back +=  "(<= (" + factMap.getFactName(le[i]) + ") ";
    back += ::toString(le_v[i]) + ")";
    back += "\n";
  }
  return back;
}


string Operator::flushg() {
  string back;
  for(int i = 0; i < gsize; i++) {
    back += "      ";
    back +=  "(> (" + factMap.getFactName(g[i]) + ") ";
    back += ::toString(g_v[i]) + ")";
    back += "\n";
  }
  return back;
}

string Operator::flushge() {
  string back;
  for(int i = 0; i < gesize; i++) {
    back += "      ";
    back +=  "(>= (" + factMap.getFactName(ge[i]) + ") ";
    back += ::toString(ge_v[i]) + ")";
    back += "\n";
  }
  return back;
}

string Operator::flushe() {
  string back;
  for(int i = 0; i < esize; i++) {
    back += "      ";
    back +=  "(>= (" + factMap.getFactName(e[i]) + ") ";
    back += ::toString(e_v[i]) + ")";
    back += "\n";
  }
  return back;
}






string Operator::flushdec() {
  string back;
  for(int i = 0; i < decsize; i++) {
    back += "      ";
    back +=  "(decrease (" + factMap.getFactName(dec[i]) + ") ";
    back += ::toString(dec_v[i]) + ")";
    back += "\n";
  }
  return back;
}


string Operator::flushinc() {
  string back;
  for(int i = 0; i < incsize; i++) {
    back += "      ";
    back +=  "(increase (" + factMap.getFactName(inc[i]) + ") ";
    back += ::toString(inc_v[i]) + ")";
    back += "\n";
  }
  return back;
}

string Operator::flushass() {
  string back;
  for(int i = 0; i < asssize; i++) {
    back += "      ";
    back +=  "(assign (" + factMap.getFactName(inc[i]) + ") ";
    back += ::toString(ass_v[i]) + ")";
    back += "\n";
  }
  return back;
}


string Operator::flushwhen() {
  string back;

  for(int w = 0; w < whensize; w++) {
      back += " \ncond-pre: ";
      for(int i = 0; i < cpresize[w]; i++) {
	  back += "      ";
	  if (cpre_t[w][i] == 1) back += "(at start ";
	  if (cpre_t[w][i] == 2) back += "(over all ";
	  if (cpre_t[w][i] == 3) back += "(at end ";
	  back +=  "(" + factMap.getFactName(cpre[w][i])+ ") ";
	  if (cpre_t[w][i]) back += ")";
	  back += "\n";
      }
      back += " cond-pdel: ";
      for(int i = 0; i < cpdelsize[w]; i++) {
	  back += "      ";
	  if (cpdel_t[w][i] == 1) back += "(at start ";
	  if (cpdel_t[w][i] == 2) back += "(over all ";
	  if (cpdel_t[w][i] == 3) back += "(at end ";
	  back +=  " (not (" + factMap.getFactName(cpdel[w][i]) + ")) ";
	  if (cpdel_t[w][i]) back += ")";
	  back += "\n";
      }
      back += " cond-add: ";
      for(int i = 0; i < caddsize[w]; i++) {
	  back += "      ";
	  if (cadd_t[w][i] == 1) back += "(at start ";
	  if (cadd_t[w][i] == 2) back += "(over all ";
	  if (cadd_t[w][i] == 3) back += "(at end ";
	  back +=  "(" + factMap.getFactName(cadd[w][i]) + ") ";
	  if (cadd_t[w][i]) back += ")";
	  back += "\n";
      }
      back += " cond-del: ";
      for(int i = 0; i < cdelsize[w]; i++) {
	  back += "      ";
	  if (cdel_t[w][i] == 1) back += "(at start ";
	  if (cdel_t[w][i] == 2) back += "(over all ";
	  if (cdel_t[w][i] == 3) back += "(at end ";
	  back +=  "(not (" + factMap.getFactName(cdel[w][i]) + ")) ";
	  if (cdel_t[w][i]) back += ")";
	  back += "\n";
      }
      
  }



  return back;
}



string Operator::toString() {
  string back;
  back += factMap.getActionName(number);
/*
//  back += derived ? " derived " : "";
    back += durative ? " cost " : "";
    back += durative ? ::toString(duration) : "";
    back += " \npre: "+ flushpre();
    back += " pdel: "+ flushpdel();
    back += " l: "+ flushl();
    back += " le: "+ flushle();
    back += " g: "+ flushg();
    back += " ge: "+ flushge();
    back += " e: "+ flushe();
    back += " ass: "+ flushass();
    back += " inc: "+ flushinc();
    back += " dec: "+ flushdec();
    back += " add: "+ flushadd();
    back += " del: "+ flushdel();
    back += "whens:" + flushwhen();
*/
  return back;
}


bool Operator::restrict(BitArray* vector) {
  int tempsize = presize;
  bool changed = false;
  presize = 0;
  for(int i = 0; i < tempsize; i++)  
    if (vector->get(pre[i]))
      pre[presize++] = pre[i];

  tempsize = addsize;
  addsize = 0;
  for(int i = 0; i < tempsize; i++)  
    if (vector->get(add[i]))
      add[addsize++] = add[i];
  changed |= (tempsize != addsize);

  tempsize = delsize;
  delsize = 0;
  for(int i = 0; i < tempsize; i++)  
    if (vector->get(del[i]))
      del[delsize++] = del[i];
  changed |= (tempsize != delsize);

  tempsize = revsize;
  revsize = 0;
  for(int i = 0; i < tempsize; i++)  
    if (vector->get(rev[i]))
      rev[revsize++] = rev[i];
  pres = presize;

  for (int w=0;w<whensize;w++) {
      int tempsize = cpresize[w];
      cpresize[w] = 0;
      for(int i = 0; i < tempsize; i++)  
	  if (vector->get(cpre[w][i]))
	      cpre[w][cpresize[w]++] = cpre[w][i];

      tempsize = caddsize[w];
      caddsize[w] = 0;
      for(int i = 0; i < tempsize; i++)  
	  if (vector->get(cadd[w][i]))
	      cadd[w][caddsize[w]++] = cadd[w][i];

      changed |= (tempsize != caddsize[w]);

      tempsize = cdelsize[w];
      cdelsize[w] = 0;
      for(int i = 0; i < tempsize; i++)  
	  if (vector->get(cdel[w][i]))
	      cdel[w][cdelsize[w]++] = cdel[w][i];

      changed |= (tempsize != cdelsize[w]);

  }
  return changed;
}
