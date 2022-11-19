#pragma once

#include <vector>

#include "Vector3.h"

class BernsteinPolynome
{
public:
	BernsteinPolynome(size_t count);
	virtual ~BernsteinPolynome(void);

	void Init( size_t count );

	double PolyN( size_t v, double x) const { return Poly(Size()-1, v, x); }
	double Poly(size_t n, size_t v, double x) const ;

	size_t Size() const {return mBinomCoeff.size();}

protected:
	// Precomputation of the coeff
	std::vector<double> mBinomCoeff;
};

class BernsteinTensor
{
public:
	BernsteinTensor( );
	virtual ~BernsteinTensor(void){}

	void Init( const std::vector< std::vector< std::vector<TVector3> > >& controlPoints );

	TVector3 Apply( const TVector3& point ) const;

protected:

	std::vector< std::vector< std::vector<TVector3> > >  mControlPoints;
	BernsteinPolynome mUPoly;
	BernsteinPolynome mVPoly;
	BernsteinPolynome mWPoly;
};
