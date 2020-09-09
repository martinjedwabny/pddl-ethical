#ifndef __NNF
#define __NNF

#include "RPGBuilder.h"
#include "globals.h"
#include <string.h>

namespace Planner {

class NNFNode;
class NNFContainerNode;
class NNFNode_AND;
class NNFNode_OR;
class NNFNode_Literal;
class NNFNode_NOT_Literal;
class NNFNode_Numeric;
class NNFNode_NOT_Numeric;
class NNFNode_RPGNumeric;
class NNFNode_NOT_RPGNumeric;

enum NNFNodeType {  	NNF_EQ_AND, NNF_EQ_OR,
			NNF_EQ_LIT, NNF_EQ_NOTLIT, NNF_EQ_NUM, NNF_EQ_NOTNUM, NNF_EQ_RPGNUM, NNF_EQ_NOTRPGNUM,
			NNF_EQ_LITSTART, NNF_EQ_LITOVERALL, NNF_EQ_LITEND,
			NNF_EQ_NOTLITSTART, NNF_EQ_NOTLITOVERALL,NNF_EQ_NOTLITEND,
			NNF_EQ_NUMSTART, NNF_EQ_NUMOVERALL, NNF_EQ_NUMEND,
			NNF_EQ_NOTNUMSTART, NNF_EQ_NOTNUMOVERALL, NNF_EQ_NOTNUMEND,
			NNF_EQ_RPGNUMSTART, NNF_EQ_RPGNUMOVERALL, NNF_EQ_RPGNUMEND,
			NNF_EQ_NOTRPGNUMSTART, NNF_EQ_NOTRPGNUMOVERALL, NNF_EQ_NOTRPGNUMEND};

class NNFVisitor {

protected:

	NNFVisitor() {};

public:

	virtual ~NNFVisitor() {};

	virtual void visit_AND(NNFNode_AND *);
	virtual void visit_OR(NNFNode_OR *);
	virtual void visit_NOT_Literal(NNFNode_NOT_Literal *);
	virtual void visit_Literal(NNFNode_Literal *);
	virtual void visit_NOT_Numeric(NNFNode_NOT_Numeric *);
	virtual void visit_Numeric(NNFNode_Numeric *);
	virtual void visit_NOT_RPGNumeric(NNFNode_NOT_RPGNumeric *);
	virtual void visit_RPGNumeric(NNFNode_RPGNumeric *);


};

class NNFNode {

protected:
	NNFNode() {};
	
public:

	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const = 0;

	typedef list<pair<NNFNodeType,int> > key;
	virtual ~NNFNode() {};
	virtual void visit(NNFVisitor *) = 0;
	virtual NNFNode * negate() = 0;
	virtual void getKey(key & dest) const {
		if (this) prefixForComparisons(dest);
	}

    virtual ostream & write(ostream & o) = 0;
};

class NNFContainerNode : public NNFNode {

protected:
	int csize;
	list<NNFNode*> children;

	NNFContainerNode() : csize(0) {};
	void clear() { children.clear(); csize = 0; };
	
	virtual NNFNodeType getType() const = 0;


public:
	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {

		dest.push_back(make_pair(getType(),csize));

		list<NNFNode*>::const_iterator cItr = children.begin();
		const list<NNFNode*>::const_iterator cEnd = children.end();

		for (; cItr != cEnd; ++cItr) (*cItr)->prefixForComparisons(dest);
	};

	typedef list<NNFNode*>::iterator iterator;
	typedef list<NNFNode*>::const_iterator const_iterator;

	virtual ~NNFContainerNode() {
		list<NNFNode*>::iterator cItr = children.begin();
		const list<NNFNode*>::iterator cEnd = children.end();

		for (; cItr != cEnd; ++cItr) delete *cItr;
	};
	virtual void addChild(NNFNode * n) {
		children.push_back(n);
		++csize;
	}

	virtual iterator eraseChild(iterator & c) {
		iterator next = c;
		++next;
		delete *c;
		children.erase(c);
		--csize;

		return next;
	}


	virtual int numberToSatisfy() = 0;

	iterator begin() { return children.begin(); };
	iterator end() { return children.end(); };

	const_iterator begin() const { return children.begin(); };
	const_iterator end() const { return children.end(); };
	
	virtual int size() const { return csize; };
	virtual bool empty() const { return children.empty(); };
	virtual const list<NNFNode*> & getChildren() const { return children; };
	virtual void stealChildren(list<NNFNode*> & c) { c.clear(); c.swap(children); csize = 0; };

};


class NNFNode_AND : public NNFContainerNode {

protected:

	virtual NNFNodeType getType() const { return NNF_EQ_AND; };
public:

	NNFNode_AND() : NNFContainerNode() {};

	virtual void visit(NNFVisitor * v) {
		v->visit_AND(this);
	};

	virtual int numberToSatisfy() {
		return csize;
	};

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
	
};

class NNFNode_OR : public NNFContainerNode {

protected:
	virtual NNFNodeType getType() const { return NNF_EQ_OR; };

public:
	NNFNode_OR() : NNFContainerNode() {};

	virtual void visit(NNFVisitor * v) {
		v->visit_OR(this);
	};

	virtual int numberToSatisfy() {
		return (csize >= 1 ? 1 : 0);
	};

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);

};

class NNFNode_Literal : public NNFNode {

protected:
	Literal * lit;
	VAL::time_spec ts;


public:

	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {
		switch(ts) {
			case (VAL::E_AT_START):
			{
				dest.push_back(make_pair(NNF_EQ_LITSTART,lit->getGlobalID()));
				break;
			}
			case (VAL::E_OVER_ALL):
			{
                dest.push_back(make_pair(NNF_EQ_LITOVERALL,lit->getGlobalID()));
				break;
			}
			case (VAL::E_AT_END):
			{
                dest.push_back(make_pair(NNF_EQ_LITEND,lit->getGlobalID()));
				break;
			}
			default:
			{
                dest.push_back(make_pair(NNF_EQ_LIT,lit->getGlobalID()));
				break;
			}
		}
	};

	NNFNode_Literal(Literal * l, const VAL::time_spec & t) : lit(l), ts(t) {};
	NNFNode_Literal(Literal * l) : lit(l), ts(VAL::E_AT) {};

	virtual ~NNFNode_Literal() {
		lit = 0;
	};

	virtual void visit(NNFVisitor * v) {
		v->visit_Literal(this);
	};

	virtual Literal* getLiteral() const { return lit; };
	virtual VAL::time_spec getTS() const { return ts; };

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
};

class NNFNode_NOT_Literal : public NNFNode_Literal {

public:

	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {
		switch(ts) {
			case (VAL::E_AT_START):
			{
                dest.push_back(make_pair(NNF_EQ_NOTLITSTART,lit->getGlobalID()));
				break;
			}
			case (VAL::E_OVER_ALL):
			{
                dest.push_back(make_pair(NNF_EQ_NOTLITOVERALL,lit->getGlobalID()));
				break;
			}
			case (VAL::E_AT_END):
			{
                dest.push_back(make_pair(NNF_EQ_NOTLITEND,lit->getGlobalID()));
				break;
			}
			default:
			{
                dest.push_back(make_pair(NNF_EQ_NOTLIT,lit->getGlobalID()));
				break;
			}
		}
	};

	NNFNode_NOT_Literal(Literal * l, const VAL::time_spec & t) : NNFNode_Literal(l,ts) {};
	NNFNode_NOT_Literal(Literal * l) : NNFNode_Literal(l) {};

	virtual void visit(NNFVisitor * v) {
		v->visit_NOT_Literal(this);
	};

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
};


class NNFNode_Numeric : public NNFNode {
protected:
	RPGBuilder::NumericPrecondition * pre;
	VAL::time_spec ts;


public:

	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {
		switch(ts) {
			case (VAL::E_AT_START):
			{
				dest.push_back(make_pair(NNF_EQ_NUMSTART,-1));
				break;
			}
			case (VAL::E_OVER_ALL):
			{
				dest.push_back(make_pair(NNF_EQ_NUMOVERALL,-1));
				break;
			}
			case (VAL::E_AT_END):
			{
				dest.push_back(make_pair(NNF_EQ_NUMEND,-1));
				break;
			}
			default:
			{
				dest.push_back(make_pair(NNF_EQ_NUM,-1));
				break;
			}
		}
	};

	NNFNode_Numeric(RPGBuilder::NumericPrecondition * p, const VAL::time_spec & t) : pre(p), ts(t) {};
	NNFNode_Numeric(RPGBuilder::NumericPrecondition * p) : pre(p), ts(VAL::E_AT) {};
	virtual ~NNFNode_Numeric() {
		delete pre;
		pre = 0;
	};

	virtual void visit(NNFVisitor * v) {
		v->visit_Numeric(this);
	};

	virtual RPGBuilder::NumericPrecondition* getPre() { return pre; };
	virtual VAL::time_spec getTS() const { return ts; };

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
};

class NNFNode_NOT_Numeric : public NNFNode_Numeric {

public:
	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {

		switch(ts) {
			case (VAL::E_AT_START):
			{
				dest.push_back(make_pair(NNF_EQ_NOTNUMSTART,-1));
				break;
			}
			case (VAL::E_OVER_ALL):
			{
				dest.push_back(make_pair(NNF_EQ_NOTNUMOVERALL,-1));
				break;
			}
			case (VAL::E_AT_END):
			{
				dest.push_back(make_pair(NNF_EQ_NOTNUMEND,-1));
				break;
			}
			default:
			{
				dest.push_back(make_pair(NNF_EQ_NOTNUM,-1));
				break;
			}
		}
	}

	NNFNode_NOT_Numeric(RPGBuilder::NumericPrecondition * p, const VAL::time_spec & t) : NNFNode_Numeric(p,t) {};
	NNFNode_NOT_Numeric(RPGBuilder::NumericPrecondition * p) : NNFNode_Numeric(p) {};

	virtual void visit(NNFVisitor * v) {
		v->visit_NOT_Numeric(this);
	};

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
};

class NNFNode_RPGNumeric : public NNFNode {
protected:
	int pre;
	VAL::time_spec ts;


public:

	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {

		switch(ts) {
			case (VAL::E_AT_START):
			{
				dest.push_back(make_pair(NNF_EQ_RPGNUMSTART,pre));
				break;
			}
			case (VAL::E_OVER_ALL):
			{
				dest.push_back(make_pair(NNF_EQ_RPGNUMOVERALL,pre));
				break;
			}
			case (VAL::E_AT_END):
			{
				dest.push_back(make_pair(NNF_EQ_RPGNUMEND,pre));
				break;
			}
			default:
			{
				dest.push_back(make_pair(NNF_EQ_RPGNUM,pre));
				break;
			}
		}
	}

	NNFNode_RPGNumeric(const int & p, const VAL::time_spec & t) : pre(p), ts(t) {};
	NNFNode_RPGNumeric(const int & p) : pre(p), ts(VAL::E_AT) {};
	virtual ~NNFNode_RPGNumeric() {
		pre = -1;
	};

	virtual void visit(NNFVisitor * v) {
		v->visit_RPGNumeric(this);
	};

	virtual const int & getPre() const { return pre; };
	virtual const VAL::time_spec & getTS() const { return ts; };

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
};

class NNFNode_NOT_RPGNumeric : public NNFNode_RPGNumeric {

public:
	virtual void prefixForComparisons(list<pair<NNFNodeType,int> > & dest) const {

		switch(ts) {
			case (VAL::E_AT_START):
			{
				dest.push_back(make_pair(NNF_EQ_NOTRPGNUMSTART,pre));
				break;
			}
			case (VAL::E_OVER_ALL):
			{
				dest.push_back(make_pair(NNF_EQ_NOTRPGNUMOVERALL,pre));
				break;
			}
			case (VAL::E_AT_END):
			{
				dest.push_back(make_pair(NNF_EQ_NOTRPGNUMEND,pre));
				break;
			}
			default:
			{
				dest.push_back(make_pair(NNF_EQ_NOTRPGNUM,pre));
				break;
			}
		}
	}


	NNFNode_NOT_RPGNumeric(const int & p, const VAL::time_spec & t) : NNFNode_RPGNumeric(p,t) {};
	NNFNode_NOT_RPGNumeric(const int & p) : NNFNode_RPGNumeric(p) {};

	virtual void visit(NNFVisitor * v) {
		v->visit_NOT_RPGNumeric(this);
	};

	virtual NNFNode * negate();
    virtual ostream & write(ostream & o);
};



class NNF_Flat {
public:
	struct Cell {
		Literal* lit;
		int num;
		bool polarity;
		Cell(Literal * l, const bool & b) : lit(l), num(-1), polarity(b){};
		Cell(const int & n, const bool & b) : lit(0), num(n),polarity(b) {};
		Cell() : lit(0), num(-1) {};
		bool isCell() const { return ((lit != 0) || (num != -1)); };
	};

private:
	const int unsatSize;
	int * const unsatReset;
	int * const unsat;
	int * const fragilityTrue;
	int * const fragilityFalse;
	
    bool * const cellIsAnd;
    
	const int cellCount;
	int * const parentID;
	Cell * const cells;

	const int resetSize;

	static int currCell;

public:
	NNF_Flat(const list<pair<int,int> > & usr, const list<pair<Cell, int> > & cellAndParent, const list<bool> & r);

	~NNF_Flat() {
		delete [] unsatReset;
		delete [] unsat;
		delete [] parentID;
		delete [] cells;
	}

    const int & getCellCount() const {
        return cellCount;
    }

    const int getInteriorNodeCount() const {
        return resetSize / sizeof(int);
    }
    
    
    const int * getParentIDs() const {
        return parentID;
    }
    
    const Cell * const getCells() const {
        return cells;
    }
    
    const int * getCurrUnsat() const {
        return unsat;
    }

    const bool * cellIsAnAnd() const {
        return cellIsAnd;
    }

	inline void reset() { memcpy(unsat,unsatReset,resetSize); memset(fragilityTrue,0,resetSize); memset(fragilityFalse,0,resetSize);};

	inline void satisfy(const int & i) {
		for (currCell = parentID[i]; currCell != -1 && (--(unsat[currCell]) == 0); currCell = parentID[currCell]) ;
	
	};

	inline void unsatisfy(const int & i) {		
		for (currCell = parentID[i]; currCell != -1 && (++(unsat[currCell]) == 1); currCell = parentID[currCell]) ;
	};

	inline void satisfyFragile(const int & i) {
		currCell = parentID[i];
		while (currCell != -1) {
			++fragilityTrue[currCell];
			if (--(unsat[currCell]) == 0) {
				currCell = parentID[currCell];
			} else {
				return;
			}
		}
	
	};

	inline void unsatisfyFragile(const int & i) {
		currCell = parentID[i];
		while (currCell != -1) {
			++fragilityFalse[currCell];
			if (++(unsat[currCell]) == 1) {
				currCell = parentID[currCell];
			} else {
				return;
			}
		}	
	};

	inline void satisfyNotFragile(const int & i) {
		currCell = parentID[i];
		while (currCell != -1) {
			if (--(unsat[currCell]) == 0) {
				currCell = parentID[currCell];
			} else if (fragilityTrue[currCell] + unsat[currCell] == 0) {
				currCell = parentID[currCell];
				while (currCell != -1) {
					if (--fragilityTrue[currCell] + unsat[currCell] == 0) {
						currCell = parentID[currCell];
					} else {
						return;
					}
				}
			} else {
				return;
			}
		}
	
	};

	inline void unsatisfyNotFragile(const int & i) {
		currCell = parentID[i];
		while (currCell != -1) {
			if (++(unsat[currCell]) == 1) {
				currCell = parentID[currCell];
			} else if (unsat[currCell] - fragilityFalse[currCell] == 1) {
				currCell = parentID[currCell];
				while (currCell != -1) {
					if (unsat[currCell] - --fragilityFalse[currCell] == 1) {
						currCell = parentID[currCell];
					} else {
						return;
					}
				}
			} else {
				return;
			}
		}	
	};

	inline void isNowNotFragileSatisfied(const int & i) {
		currCell = parentID[i];
		while (currCell != -1 && --fragilityTrue[currCell] + unsat[currCell] == 0) {
			currCell = parentID[currCell];			
		}
	
	};

	inline void isNowNotFragileUnsatisfied(const int & i) {
		currCell = parentID[i];
		while (currCell != -1 && unsat[currCell] - --fragilityFalse[currCell] == 1) {
			currCell = parentID[currCell];
		}
	};


	inline bool isSatisfied() const { return (unsat[0] <= 0); };
	inline bool isFragile() const {
		if (unsat[0] <= 0) {
			return (unsat[0] + fragilityTrue[0] > 0);
		} else {
			return (unsat[0] - fragilityFalse[0] <= 0);
		}
	}

	void write(ostream & o) const;

	template<typename T>
	void fillDependencyTables(vector<list<LiteralCellDependency<T> > > & litTable,vector<list<LiteralCellDependency<T> > > & negativeLitTable, vector<list<LiteralCellDependency<T> > > & numTable, vector<list<LiteralCellDependency<T> > > & negativeNumTable, list<int> & llist, list<int> & nllist, list<int> & nlist, list<int> & nnlist, const T & dest) {
		for (int i = 0; i < cellCount; ++i) {
			if (cells[i].lit) {
				if (cells[i].polarity) {
					litTable[cells[i].lit->getStateID()].push_back(LiteralCellDependency<T>(dest,i));
					llist.push_back(cells[i].lit->getStateID());
				} else {
					negativeLitTable[cells[i].lit->getStateID()].push_back(LiteralCellDependency<T>(dest,i));
					nllist.push_back(cells[i].lit->getStateID());
				}
			}
			if (cells[i].num != -1) {
				if (cells[i].polarity) {
					numTable[cells[i].num].push_back(LiteralCellDependency<T>(dest,i));
					nlist.push_back(cells[i].num);
				} else {
					negativeNumTable[cells[i].num].push_back(LiteralCellDependency<T>(dest,i));
					nllist.push_back(cells[i].num);
				}
			}
		}
	};
};

ostream & operator <<(ostream & o, const NNF_Flat & f);

class NNFUtils {

private:

	template<typename T>
	class NNFBuildDep0 : public NNFVisitor {

	protected:

		vector<list<LiteralCellDependency<T> > > & depTable;

		T & dest;

	public:

		NNFBuildDep0(vector<list<LiteralCellDependency<T> > > & table, T & destIn) : depTable(table), dest(destIn) {};

		virtual void visit_Literal(NNFNode_Literal * litNode) {
			int dummy = -1;
			depTable[litNode->getLiteral()->getStateID()].push_back(LiteralCellDependency<T>(dest,dummy));
		}


	};

	class NNFRPGNumericSubstituter;

public:
	static pair<NNFNode*,bool> buildNNF(VAL::TypeChecker * tc, VAL::FastEnvironment * fe, VAL::goal * g);
	static NNF_Flat * flattenNNF(NNFNode* const root);
	static NNFNode* simplifyNNF(NNFNode * const root);
	static pair<NNFNode*,bool> pruneStaticLiterals(NNFNode* const root,vector<pair<bool,bool> > & staticLiterals);
	static pair<NNFNode*,bool> pruneStaticNumerics(NNFNode* const root,vector<pair<bool,bool> > & staticNumerics);
	static pair<NNFNode*,bool> pruneUnreachable(NNFNode* const root,LiteralSet & initialState);
	static pair<NNFNode*,bool> substituteRPGNumerics(NNFNode * const root, RPGBuilder::BuildingNumericPreconditionData & commonData);

	template<typename T>
	static void buildLiteralsToPreconditions(NNFNode* const root, vector<list<LiteralCellDependency<T> > > & litTable, T & dest) {

		NNFBuildDep0<T> c(litTable,dest);
		root->visit(&c);
	};

	template<typename T>
	static void buildLiteralsToPreconditions(NNF_Flat* const root, vector<list<LiteralCellDependency<T> > > & litTable, vector<list<LiteralCellDependency<T> > > & negativeLitTable, vector<list<LiteralCellDependency<T> > > & numTable, vector<list<LiteralCellDependency<T> > > & negNumTable, list<int> & thisToLit, list<int> & thisToNegLit, list<int> & thisToNum, list<int> & thisToNegNum, const T & dest) {
		root->fillDependencyTables<T>(litTable,negativeLitTable,
						numTable,negNumTable,
						thisToLit,thisToNegLit,
						thisToNum,thisToNegNum,
						dest);
	};

};

};

#endif
