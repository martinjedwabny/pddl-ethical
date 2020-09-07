#ifndef FUNCTION_OBJECT_H
#define FUNCTION_OBJECT_H

BEGIN_HSPS_NAMESPACE

template<class D, class R> class MonadicFunction {
 public:
  virtual ~MonadicFunction();
  virtual R operator()(const D& a) const = 0;
};

template<class D, class R>
MonadicFunction<D,R>::~MonadicFunction()
{
  // nada
}

template<class D, class R> class DyadicFunction {
 public:
  virtual ~DyadicFunction();
  virtual R operator()(const D& a, const D& b) const = 0;
};

template<class D, class R>
DyadicFunction<D,R>::~DyadicFunction()
{
  // nada
}

END_HSPS_NAMESPACE

#endif
