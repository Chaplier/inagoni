#include "BernsteinTensor.h"

// http://en.wikipedia.org/wiki/Binomial_coefficient
// C n,k
size_t choose(size_t n, size_t k) 
{
    if (k > n)
        return 0;

    if (k > n/2)
        k = n-k; // Take advantage of symmetry

	double accum = 1;
    for (size_t i = 1; i <= k; i++)
         accum = accum * (n-k+i) / i;

    return accum + 0.5; // avoid rounding error*/
}

std::vector<double> ComputeCoeff( size_t count )
{
	std::vector<double> result(count);

	size_t halfCount = (count+1)/2;
	for(size_t i=0 ; i<halfCount ; i++ )
	{
		result[i] = choose(count-1, i);
		result[count-1-i] = result[i];
	}

	return result;
}

BernsteinPolynome::BernsteinPolynome(size_t count)
{
	mBinomCoeff = ComputeCoeff( count );
}

BernsteinPolynome::~BernsteinPolynome(void)
{
}

void BernsteinPolynome::Init( size_t count )
{
	mBinomCoeff = ComputeCoeff( count );
}

double PowerInt(double a, size_t b) { 
    if (b<=0) return 1; 
    else if (b==1) return a; 
    else { 
        return a*PowerInt(a,b-1); 
    } 
} 

double BernsteinPolynome::Poly(size_t n, size_t v, double x) const
{
	// bernstein polynomial expansion
	//return mBinomCoeff[v] * pow(x,(double)v) *  pow((1.0f-x),(double)(n-v));
	// pow is slow, try to speed up things:
	return mBinomCoeff[v] * PowerInt(x,v) *  PowerInt((1-x),(n-v));
}

BernsteinTensor::BernsteinTensor()
: mUPoly(0),mVPoly(0),mWPoly(0) 
{
}

void BernsteinTensor::Init( const std::vector< std::vector< std::vector<TVector3> > >& controlPoints)
{
	mControlPoints = controlPoints;
	mUPoly.Init( mControlPoints.size() );
	mVPoly.Init( mControlPoints[0].size() );
	mWPoly.Init( mControlPoints[0][0].size() );
}

TVector3 BernsteinTensor::Apply( const TVector3& point ) const
{
	TVector3 result(0.0f,0.0f,0.0f);
	const size_t uCount = mUPoly.Size();
	const size_t vCount = mVPoly.Size();
	const size_t wCount = mWPoly.Size();
	for(size_t i = 0; i < uCount; i++){
		
		TVector3 tM(0.0f,0.0f,0.0f);
		for(size_t j = 0; j < vCount; j++){
			
			TVector3 tK(0.0f,0.0f,0.0f);
			for(size_t k = 0; k < wCount; k++){
				tK += mWPoly.PolyN(k,point.z) * mControlPoints[i][j][k];
			}
			tM += mVPoly.PolyN(j,point.y) * tK;
		}
		result += mUPoly.PolyN(i,point.x) * tM;
	}

	return result;
}