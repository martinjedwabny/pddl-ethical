
#include "logic.h"
#include "enumerators.h"
#include <sstream>

BEGIN_HSPS_NAMESPACE

void new_variable_vec
(PDDL_Base* base, index_type n, PDDL_Base::variable_vec& v)
{
  while (n > 0) {
    v.append((PDDL_Base::VariableSymbol*)base->gensym(PDDL_Base::sym_variable, "?v", 0));
    n -= 1;
  }
}

PDDL_Base::AFormula* new_atomic_formula
(PDDL_Base* base, PDDL_Base::PredicateSymbol* p)
{
  PDDL_Base::AFormula* a = new PDDL_Base::AFormula(p);
  a->param.set_length(p->param.length());
  for (index_type k = 0; k < p->param.length(); k++) {
    a->param[k] = base->gensym(PDDL_Base::sym_variable, "?v", 0);
  }
}

void create_predicate_substitution
(const char* suffix,
 lvector<PDDL_Base::PredicateSymbol*> preds,
 PDDL_Base::symbol_pair_vec& s)
{
  for (index_type k = 0; k < preds.length(); k++) {
    std::ostringstream s_name;
    s_name << preds[k]->print_name << "-" << suffix;
    PDDL_Base::PredicateSymbol* s_pred =
      new PDDL_Base::PredicateSymbol(strdup(s_name.str().c_str()));
    s_pred->param = preds[k]->param;
    s_pred->pos_pre = preds[k]->pos_pre;
    s_pred->neg_pre = preds[k]->neg_pre;
    s_pred->added = preds[k]->added;
    s_pred->deleted = preds[k]->deleted;
    s_pred->locked = preds[k]->locked;
    s_pred->modded = preds[k]->modded;
    s_pred->init = preds[k]->init;
    s_pred->pos_goal = preds[k]->pos_goal;
    s_pred->neg_goal = preds[k]->neg_goal;
    s_pred->pos_prop = preds[k]->pos_prop;
    s_pred->neg_prop = preds[k]->neg_prop;
    s_pred->irr_ins = preds[k]->irr_ins;
    s.append(PDDL_Base::symbol_pair(preds[k], s_pred));
  }
}

PDDL_Base::Formula* make_unifying_formula(PDDL_Base::symbol_pair_vec& u)
{
  if (u.length() == 0) {
    return new PDDL_Base::Formula(PDDL_Base::fc_true);
  }
  else if (u.length() == 1) {
    return new PDDL_Base::EqFormula(u[0].first, u[0].second);
  }
  else {
    PDDL_Base::CFormula* cf =
      new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);
    for (index_type k = 0; k < u.length(); k++)
      cf->add(new PDDL_Base::EqFormula(u[k].first, u[k].second));
    return cf;
  }
}

PDDL_Base::Formula* make_separating_formula(PDDL_Base::symbol_pair_vec& u)
{
  if (u.length() == 0) {
    return new PDDL_Base::Formula(PDDL_Base::fc_false);
  }
  else if (u.length() == 1) {
    return new PDDL_Base::NFormula(new PDDL_Base::EqFormula(u[0].first,
							    u[0].second));
  }
  else {
    PDDL_Base::CFormula* cf =
      new PDDL_Base::CFormula(PDDL_Base::fc_disjunction);
    for (index_type k = 0; k < u.length(); k++)
      cf->add(new PDDL_Base::NFormula(new PDDL_Base::EqFormula(u[k].first,
							       u[k].second)));
    return cf;
  }
}

PDDL_Base::Formula* make_atom_formula(PDDL_Base::Atom* atom)
{
  if (atom->pred->is_equality()) {
    assert(atom->param.length() == 2);
    if (atom->param[0]->sym_class == PDDL_Base::sym_variable) {
      return new PDDL_Base::EqFormula(atom->param[0], atom->param[1]);
    }
    else if (atom->param[1]->sym_class == PDDL_Base::sym_variable) {
      return new PDDL_Base::EqFormula(atom->param[1], atom->param[0]);
    }
    else {
      if (atom->param[0] == atom->param[1]) {
	return new PDDL_Base::Formula(PDDL_Base::fc_true);
      }
      else {
	return new PDDL_Base::Formula(PDDL_Base::fc_false);
      }
    }
  }
  else {
    return new PDDL_Base::AFormula(atom);
  }
}

PDDL_Base::Formula* make_context_formula(PDDL_Base::SetOf* set)
{
  if (set->pos_con.length() + set->neg_con.length() > 0) {
    PDDL_Base::CFormula* cf =
      new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);
    for (index_type k = 0; k < set->pos_con.length(); k++) {
      cf->add(make_atom_formula(set->pos_con[k]));
    }
    for (index_type k = 0; k < set->neg_con.length(); k++) {
      cf->add(new PDDL_Base::NFormula(make_atom_formula(set->neg_con[k])));
    }
    return cf;
  }
  else {
    return new PDDL_Base::Formula(PDDL_Base::fc_true);
  }
}

PDDL_Base::Formula* make_universal_formula(PDDL_Base::SetOf* set)
{
  PDDL_Base::QFormula* qf = new PDDL_Base::QFormula(PDDL_Base::fc_universal);
  for (index_type k = 0; k < set->param.length(); k++)
    qf->add(set->param[k]);
  PDDL_Base::CFormula* df =
    new PDDL_Base::CFormula(PDDL_Base::fc_disjunction);
  for (index_type k = 0; k < set->pos_con.length(); k++) {
    df->add(new PDDL_Base::NFormula(make_atom_formula(set->pos_con[k])));
  }
  for (index_type k = 0; k < set->neg_con.length(); k++) {
    df->add(make_atom_formula(set->neg_con[k]));
  }
  PDDL_Base::CFormula* cf =
    new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);
  for (index_type k = 0; k < set->pos_atoms.length(); k++) {
    cf->add(make_atom_formula(set->pos_atoms[k]));
  }
  for (index_type k = 0; k < set->neg_atoms.length(); k++) {
    cf->add(new PDDL_Base::NFormula(make_atom_formula(set->neg_atoms[k])));
  }
  df->add(cf);
  qf->f = df;
  return qf;
}

PDDL_Base::Formula* action_precondition(PDDL_Base::ActionSymbol* act)
{
  if (act->pos_pre.empty() && act->neg_pre.empty() && act->set_pre.empty()) {
    return new PDDL_Base::Formula(PDDL_Base::fc_true);
  }
  PDDL_Base::CFormula* pre =
    new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);
  for (index_type k = 0; k < act->pos_pre.length(); k++) {
    pre->add(make_atom_formula(act->pos_pre[k]));
  }
  for (index_type k = 0; k < act->neg_pre.length(); k++) {
    pre->add(new PDDL_Base::NFormula(make_atom_formula(act->neg_pre[k])));
  }
  for (index_type k = 0; k < act->set_pre.length(); k++) {
    pre->add(make_universal_formula(act->set_pre[k]));
  }
  return pre;
}

PDDL_Base::Formula* action_adds_atom
(PDDL_Base::ActionSymbol* act, PDDL_Base::Atom* atom)
{
  for (index_type k = 0; k < act->adds.length(); k++) {
    if (act->adds[k]->equals(*atom))
      return new PDDL_Base::Formula(PDDL_Base::fc_true);
  }
  PDDL_Base::formula_vec d;
  for (index_type k = 0; k < act->adds.length(); k++) {
    PDDL_Base::symbol_pair_vec u;
    if (act->adds[k]->unify(atom, u)) {
      d.append(make_unifying_formula(u));
    }
  }
  for (index_type k = 0; k < act->set_eff.length(); k++)
    for (index_type i = 0; i < act->set_eff[k]->pos_atoms.length(); i++) {
      PDDL_Base::symbol_pair_vec u;
      if (act->set_eff[k]->pos_atoms[i]->unify(atom, u)) {
	PDDL_Base::Formula* cx = make_context_formula(act->set_eff[k]);
	PDDL_Base::Formula* eq = make_unifying_formula(u);
	PDDL_Base::Formula* cf =
	  new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, eq, cx);
	PDDL_Base::Formula* qf =
	  new PDDL_Base::QFormula(PDDL_Base::fc_existential,
				  act->set_eff[k]->param,
				  cf);
	d.append(qf);
      }
    }
  for (index_type k = 0; k < act->cond_eff.length(); k++)
    for (index_type i = 0; i < act->cond_eff[k]->pos_atoms.length(); i++) {
      PDDL_Base::symbol_pair_vec u;
      if (act->cond_eff[k]->pos_atoms[i]->unify(atom, u)) {
	PDDL_Base::Formula* cx = make_context_formula(act->cond_eff[k]);
	PDDL_Base::Formula* eq = make_unifying_formula(u);
	PDDL_Base::Formula* cf =
	  new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, eq, cx);
	PDDL_Base::Formula* qf =
	  new PDDL_Base::QFormula(PDDL_Base::fc_existential,
				  act->cond_eff[k]->param,
				  cf);
	d.append(qf);
      }
    }
  if (d.length() == 0) {
    return new PDDL_Base::Formula(PDDL_Base::fc_false);
  }
  else if (d.length() == 1) {
    return d[0];
  }
  else {
    PDDL_Base::CFormula* df =
      new PDDL_Base::CFormula(PDDL_Base::fc_disjunction);
    df->add(d);
    return df;
  }
}

PDDL_Base::Formula* action_deletes_atom
(PDDL_Base::ActionSymbol* act, PDDL_Base::Atom* atom)
{
  for (index_type k = 0; k < act->dels.length(); k++) {
    if (act->dels[k]->equals(*atom))
      return new PDDL_Base::Formula(PDDL_Base::fc_true);
  }
  PDDL_Base::formula_vec d;
  for (index_type k = 0; k < act->dels.length(); k++) {
    PDDL_Base::symbol_pair_vec u;
    if (act->dels[k]->unify(atom, u)) {
      d.append(make_unifying_formula(u));
    }
  }
  for (index_type k = 0; k < act->set_eff.length(); k++)
    for (index_type i = 0; i < act->set_eff[k]->neg_atoms.length(); i++) {
      PDDL_Base::symbol_pair_vec u;
      if (act->set_eff[k]->neg_atoms[i]->unify(atom, u)) {
	PDDL_Base::Formula* cx = make_context_formula(act->set_eff[k]);
	PDDL_Base::Formula* eq = make_unifying_formula(u);
	PDDL_Base::Formula* cf =
	  new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, eq, cx);
	PDDL_Base::Formula* qf =
	  new PDDL_Base::QFormula(PDDL_Base::fc_existential,
				  act->set_eff[k]->param,
				  cf);
	d.append(qf);
      }
    }
  for (index_type k = 0; k < act->cond_eff.length(); k++)
    for (index_type i = 0; i < act->cond_eff[k]->neg_atoms.length(); i++) {
      PDDL_Base::symbol_pair_vec u;
      if (act->cond_eff[k]->neg_atoms[i]->unify(atom, u)) {
	PDDL_Base::Formula* cx = make_context_formula(act->cond_eff[k]);
	PDDL_Base::Formula* eq = make_unifying_formula(u);
	PDDL_Base::Formula* cf =
	  new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, eq, cx);
	PDDL_Base::Formula* qf =
	  new PDDL_Base::QFormula(PDDL_Base::fc_existential,
				  act->cond_eff[k]->param,
				  cf);
	d.append(qf);
      }
    }
  if (d.length() == 0) {
    return new PDDL_Base::Formula(PDDL_Base::fc_false);
  }
  else if (d.length() == 1) {
    return d[0];
  }
  else {
    PDDL_Base::CFormula* df =
      new PDDL_Base::CFormula(PDDL_Base::fc_disjunction);
    df->add(d);
    return df;
  }
}

PDDL_Base::Formula* make_step_formula
(PDDL_Base* base,
 PDDL_Base::ActionSymbol* act,
 lvector<PDDL_Base::PredicateSymbol*> preds,
 PDDL_Base::symbol_pair_vec& s0,
 PDDL_Base::symbol_pair_vec& s1)
{
  PDDL_Base::CFormula* f = new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);

  // preconditions hold in s0
  PDDL_Base::Formula* pre = action_precondition(act);
  pre->rename_predicates_1(s0);
  f->add(pre);

  for (index_type k = 0; k < preds.length(); k++) {
    // af holds in s1 iff cf holds in s0
    PDDL_Base::variable_vec new_vars;
    new_variable_vec(base, preds[k]->param.length(), new_vars);
    PDDL_Base::AFormula* af = new PDDL_Base::AFormula(preds[k], new_vars);
    PDDL_Base::Formula* cf =
      new PDDL_Base::CFormula(PDDL_Base::fc_disjunction, action_adds_atom(act, af), new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, new PDDL_Base::NFormula(action_deletes_atom(act, af)), af));
    af->rename_predicates_1(s1);
    cf->rename_predicates_1(s0);
    PDDL_Base::Formula* iff =
      new PDDL_Base::BFormula(PDDL_Base::fc_equivalence, af, cf);
    // quantify new variables in iff
    f->add(new PDDL_Base::QFormula(PDDL_Base::fc_universal, new_vars, iff));
  }

  return new PDDL_Base::QFormula(PDDL_Base::fc_universal, act->param, f);
}

PDDL_Base::Formula*
regress(PDDL_Base::Formula* f, PDDL_Base::ActionSymbol* act)
{
  switch (f->fc) {
  case PDDL_Base::fc_false:
  case PDDL_Base::fc_true:
  case PDDL_Base::fc_equality:
    return f;
  case PDDL_Base::fc_atom:
    {
      PDDL_Base::AFormula* af = (PDDL_Base::AFormula*)f;
      return new PDDL_Base::CFormula(PDDL_Base::fc_disjunction, action_adds_atom(act, af), new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, af, new PDDL_Base::NFormula(action_deletes_atom(act, af))));
    }
  case PDDL_Base::fc_negation:
    {
      PDDL_Base::NFormula* nf = (PDDL_Base::NFormula*)f;
      return new PDDL_Base::NFormula(regress(nf->f, act));
    }
  case PDDL_Base::fc_conjunction:
  case PDDL_Base::fc_disjunction:
    {
      PDDL_Base::CFormula* cf = (PDDL_Base::CFormula*)f;
      PDDL_Base::CFormula* ncf = new PDDL_Base::CFormula(cf->fc);
      for (index_type k = 0; k < cf->parts.length(); k++) {
	ncf->add(regress(cf->parts[k], act));
      }
      return ncf;
    }
  case PDDL_Base::fc_implication:
  case PDDL_Base::fc_equivalence:
    {
      PDDL_Base::BFormula* bf = (PDDL_Base::BFormula*)f;
      return new PDDL_Base::BFormula(bf->fc, regress(bf->f1, act), regress(bf->f2, act));
    }
  case PDDL_Base::fc_universal:
  case PDDL_Base::fc_existential:
    {
      PDDL_Base::QFormula* qf = (PDDL_Base::QFormula*)f;
      return new PDDL_Base::QFormula(qf->fc, qf->vars, regress(qf->f, act));
    }
  }
  assert(0);
}

PDDL_Base::Formula*
make_verification_formula
(PDDL_Base::Formula* f, PDDL_Base* base)
{
  PDDL_Base::CFormula* vf = new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);
  for (index_type k = 0; k < base->dom_actions.length(); k++) {
    PDDL_Base::CFormula* cf = new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, f, action_precondition(base->dom_actions[k]));
    vf->add(new PDDL_Base::QFormula(PDDL_Base::fc_universal, base->dom_actions[k]->param, new PDDL_Base::BFormula(PDDL_Base::fc_implication, cf, regress(f, base->dom_actions[k]))));
  }
  return vf;
}


void make_all_ssvcs
(PDDL_Base* base, PDDL_Base::PredicateSymbol* p, PDDL_Base::formula_vec& v)
{
  SubsetEnumerator e(p->param.length());
  bool more = e.first();
  while (more) {
    index_set d;
    e.current_set(d);
    if (!d.empty()) {
      PDDL_Base::QFormula* qf =
	new PDDL_Base::QFormula(PDDL_Base::fc_universal);
      PDDL_Base::AFormula* a1 = new PDDL_Base::AFormula(p);
      for (index_type k = 0; k < p->param.length(); k++) {
	PDDL_Base::VariableSymbol* v = (PDDL_Base::VariableSymbol*)base->gensym(PDDL_Base::sym_variable, p->param[k]->print_name, p->param[k]->sym_types);
	qf->vars.append(v);
	a1->param.append(v);
      }
      PDDL_Base::AFormula* a2 = new PDDL_Base::AFormula(p);
      for (index_type k = 0; k < p->param.length(); k++) {
	PDDL_Base::VariableSymbol* v = (PDDL_Base::VariableSymbol*)base->gensym(PDDL_Base::sym_variable, p->param[k]->print_name, p->param[k]->sym_types);
	qf->vars.append(v);
	a2->param.append(v);
      }
      PDDL_Base::Formula* lhs =
	new PDDL_Base::CFormula(PDDL_Base::fc_conjunction, a1, a2);
      PDDL_Base::Formula* rhs = 0;
      if (d.length() == 1) {
	rhs = new PDDL_Base::EqFormula(a1->param[d[0]], a2->param[d[0]]);
      }
      else {
	PDDL_Base::CFormula* cf =
	  new PDDL_Base::CFormula(PDDL_Base::fc_conjunction);
	for (index_type k = 0; k < d.length(); k++) {
	  cf->add(new PDDL_Base::EqFormula(a1->param[d[k]], a2->param[d[k]]));
	}
	rhs = cf;
      }
      qf->f = new PDDL_Base::BFormula(PDDL_Base::fc_implication, lhs, rhs);
      v.append(qf);
    }
    more = e.next();
  }
}


void make_simple_categories
(PDDL_Base* base, PDDL_Base::TypeSymbol* t, PDDL_Base::VariableSymbol* voft,
 PDDL_Base::formula_vec& v)
{
  for (index_type p = 0; p < base->dom_predicates.length(); p++) {
    PDDL_Base::PredicateSymbol* pred = base->dom_predicates[p];
    for (index_type k = 0; k < pred->param.length(); k++)
      if ((t == 0) || (t->subtype_or_equal(pred->param[k]->sym_types))) {
	if (pred->param.length() > 1) {
	  PDDL_Base::AFormula* af = new PDDL_Base::AFormula(pred);
	  af->param.set_length(pred->param.length());
	  PDDL_Base::QFormula* qf =
	    new PDDL_Base::QFormula(PDDL_Base::fc_existential);
	  qf->f = af;
	  for (index_type i = 0; i < pred->param.length(); i++) {
	    if (i == k) {
	      af->param[i] = voft;
	    }
	    else {
	      PDDL_Base::VariableSymbol* vq =
		(PDDL_Base::VariableSymbol*)
		base->gensym(PDDL_Base::sym_variable,
			     pred->param[i]->print_name,
			     pred->param[i]->sym_types);
	      af->param[i] = vq;
	      qf->vars.append(vq);
	    }
	  }
	  v.append(qf);
	}
	else {
	  PDDL_Base::AFormula* af = new PDDL_Base::AFormula(pred);
	  af->param.set_length(1);
	  af->param[0] = voft;
	  v.append(af);
	}
      }
  }
}

void make_all_bscecs
(PDDL_Base* base, PDDL_Base::TypeSymbol* t, PDDL_Base::formula_vec& v)
{
  PDDL_Base::VariableSymbol* voft =
    (PDDL_Base::VariableSymbol*)base->gensym(PDDL_Base::sym_variable, "?v", t);
  PDDL_Base::variable_vec qv(voft, 1);
  PDDL_Base::formula_vec cats(0, 0);
  make_simple_categories(base, t, voft, cats);
  if (PDDL_Base::write_trace) {
    if (t) {
      std::cerr << cats.length() << " simple categories of type "
		<< t->print_name << ": " << std::endl;
    }
    else {
      std::cerr << cats.length() << " simple categories of type object:"
		<< std::endl;
    }
    for (index_type k = 0; k < cats.length(); k++) {
      std::cerr << k + 1 << ". ";
      cats[k]->print(std::cerr);
      std::cerr << std::endl;
    }
  }
  for (index_type i = 0; i < cats.length(); i++)
    for (index_type j = 0; j < cats.length(); j++) if (i != j) {
      v.append(new PDDL_Base::QFormula
	       (PDDL_Base::fc_universal, qv,
		new PDDL_Base::BFormula
		(PDDL_Base::fc_implication,
		 cats[i],
		 new PDDL_Base::NFormula(cats[j]))));
    }
}

void make_all_scecs
(PDDL_Base* base, PDDL_Base::TypeSymbol* t, PDDL_Base::formula_vec& v)
{
  PDDL_Base::VariableSymbol* voft =
    (PDDL_Base::VariableSymbol*)base->gensym(PDDL_Base::sym_variable, "?v", t);
  PDDL_Base::variable_vec qv(voft, 1);
  PDDL_Base::formula_vec cats(0, 0);
  make_simple_categories(base, t, voft, cats);
  if (PDDL_Base::write_trace) {
    if (t) {
      std::cerr << cats.length() << " simple categories of type "
		<< t->print_name << ": " << std::endl;
    }
    else {
      std::cerr << cats.length() << " simple categories of type object:"
		<< std::endl;
    }
    for (index_type k = 0; k < cats.length(); k++) {
      std::cerr << k + 1 << ". ";
      cats[k]->print(std::cerr);
      std::cerr << std::endl;
    }
  }
  SubsetEnumerator e(cats.length());
  bool more = e.first();
  while (more) {
    if (e.current_set_size() >= 2) {
      PDDL_Base::CFormula* df =
	new PDDL_Base::CFormula(PDDL_Base::fc_disjunction);
      for (index_type k = 0; k < cats.length(); k++)
	if (e.current_set()[k])
	  df->add(new PDDL_Base::NFormula(cats[k]));
      v.append(new PDDL_Base::QFormula(PDDL_Base::fc_universal, qv, df));
    }
    more = e.next();
  }
}

END_HSPS_NAMESPACE
