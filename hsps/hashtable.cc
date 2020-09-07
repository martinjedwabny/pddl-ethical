
#include "hashtable.h"

BEGIN_HSPS_NAMESPACE

HashTable::HashTable(index_type s)
  : size(s), tab(0), hits(0), cols(0)
{
  tab = new Entry[size];
  for (index_type k = 0; k < size; k++) {
    tab[k].state = 0;
    tab[k].depth = no_such_index;
    tab[k].cost = 0;
  }
}

HashTable::~HashTable()
{
  assert(tab);
  for (index_type k = 0; k < size; k++)
    if (tab[k].state) delete tab[k].state;
  delete tab;
}

HashTable::Entry& HashTable::operator[](index_type i)
{
  assert(i < size);
  return tab[i];
}

index_type HashTable::index(State& s)
{
  return (s.hash() % size);
}

HashTable::Entry* HashTable::find(State& s)
{
  index_type h0 = s.hash();
  index_type h =  (h0 % size);
  if (tab[h].state) {
    if (tab[h].state->compare(s) == 0) {
      hits += 1;
#ifdef PRINT_EXTRA_STATS
      std::cout << "HHIT " << h << std::endl;
#endif
      return &(tab[h]);
    }
    else {
      cols += 1;
#ifdef PRINT_EXTRA_STATS
      std::cout << "HCOL " << h
		<< " / " << h0
		<< " / " << tab[h].state->hash()
		<< " : " << s
		<< " / " << *(tab[h].state)
		<< std::endl;
#endif
      return 0;
    }
  }
  else {
    miss += 1;
#ifdef PRINT_EXTRA_STATS
    std::cout << "HMISS " << h << std::endl;
#endif
    return 0;
  }
}

void HashTable::insert(State& s, NTYPE v, index_type d)
{
  index_type h = (s.hash() % size);
  if (tab[h].state) {
    delete tab[h].state;
  }
  else {
    nocc += 1;
  }
  tab[h].state = s.copy();
  tab[h].depth = d;
  tab[h].cost = v;
}

void HashTable::insert(State& s, NTYPE v)
{
  insert(s, v, s.depth());
}

void HashTable::clear()
{
  for (index_type k = 0; k < size; k++) {
    if (tab[k].state) {
      delete tab[k].state;
    }
    tab[k].state = 0;
    tab[k].depth = no_such_index;
    tab[k].cost = 0;
  }
  nocc = 0;
  hits = 0;
  cols = 0;
  miss = 0;
}

double HashTable::hit_ratio()
{
  return hits/(double)(hits + cols + miss);
}

double HashTable::HCF()
{
  return hits/(double)cols;
}

double HashTable::TUF()
{
  return nocc/(double)size;
}

END_HSPS_NAMESPACE
