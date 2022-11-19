//Macros for computing mesh Curvature.

#define erfp(x) (1+(-1.0f+(-0.282361-0.077032*x)*x)/(pow(2.71828,(double)(x*x))*pow(1+0.47047*x,3))) 

#define max(a,b)((a>b)?a:b) 

#define swapinteger(a,b) \
{int32 temp;temp=a;a=b;b=temp;};

#define swapreal32(a,b) \
{real32 temp=a;\
a=b;\
b=temp;};

#define area2(a,b,c) (((b-a)^(c-a)).GetMagnitude()) 

#define sameside(p1,p2,a,b) ( ((b-a)^(p1-a))*((b-a)^(p2-a)) >=0.0f ) 

#define intriangle(p,a,b,c) (sameside(p,a,b,c) && sameside(p,b,a,c) && sameside(p,c,a,b))  

#define COPYNORMALS \
		for (int i = 0; i < nv; i++)  \
			{ \
			TVector3 normal=amesh->fNormals[i]; \
			normal.x*=xfactor; \
			normal.y*=yfactor; \
			normal.z*=zfactor; \
			normal.Normalize(); \
			cmesh->Normals[i]=normal; \
			}; \


#define COMPUTEPOINTNORMALS \
	for (int i = 0; i < nf; i++) {\
		ThrowIfNil(amesh);\
		ThrowIfNil(cmesh);\
		TVector3 p0 = amesh->fVertices[amesh->fFacets[i].pt1];\
		TVector3 p1 = amesh->fVertices[amesh->fFacets[i].pt2];\
		TVector3 p2 = amesh->fVertices[amesh->fFacets[i].pt3];\
		p0.x*=xfactor;\
		p0.y*=yfactor;\
		p0.z*=zfactor;\
		p1.x*=xfactor;\
		p1.y*=yfactor;\
		p1.z*=zfactor;\
		p2.x*=xfactor;\
		p2.y*=yfactor;\
		p2.z*=zfactor;\
		TVector3 a = p0-p1, b= p1-p2, c= p2-p0;\
		real32 l2a = a.GetMagnitudeSquared(), l2b = b.GetMagnitudeSquared(), l2c = c.GetMagnitudeSquared();\
		TVector3 facenormal = a ^ b;\
		cmesh->Normals[amesh->fFacets[i].pt1] += facenormal * (1.0f / (l2a * l2c));\
		cmesh->Normals[amesh->fFacets[i].pt2] += facenormal * (1.0f / (l2b * l2a));\
		cmesh->Normals[amesh->fFacets[i].pt3] += facenormal * (1.0f / (l2c * l2b));\
	}\
	for (int i = 0; i < nv; i++) cmesh->Normals[i].Normalize();


//Sometimes not true math , just don't want to compute hundreds of square roots.
#define COMPUTEMINMAXCURVATURES \
	minK=0.0f;\
	maxK=0.0f;\
	minH=0.0f;\
	maxH=0.0f;\
	mink1=0.0f;\
	maxk1=0.0f;\
	mink2=0.0f;\
	maxk2=0.0f;\
	mincurve=0.0f;\
	maxcurve=0.0f;\
	minscale=0.0f;\
	maxscale=0.0f;\
	for (int i = 0; i < nv; i++) {\
		ThrowIfNil(cmesh);\
		real32 k1v=cmesh->k1s[i];\
		real32 k2v=cmesh->k2s[i];\
		real32 Kv=k1v*k2v;\
		real32 Hv=(k1v+k2v);\
		real32 curvev=k1v*k1v+k2v*k2v;\
		real32 scalev=Hv*Hv+fabs(Kv);\
		if(minK>Kv){minK=Kv;};\
		if(maxK<Kv){maxK=Kv;};\
		if(minH>Hv){minH=Hv;};\
		if(maxH<Hv){maxH=Hv;};\
		if(mink1>k1v){mink1=k1v;};\
		if(maxk1<k1v){maxk1=k1v;};\
		if(mink2>k2v){mink2=k2v;};\
		if(maxk2<k2v){maxk2=k2v;};\
		if(mincurve>curvev){mincurve=curvev;};\
		if(maxcurve<curvev){maxcurve=curvev;};\
		if(minscale>scalev){minscale=scalev;};\
		if(maxscale<scalev){maxscale=scalev;};\
			}\
	minH*=0.5f;\
	maxH*=0.5f;\
	mincurve=sqrt(0.5*mincurve);\
	maxcurve=sqrt(0.5*maxcurve);\
	minscale=sqrt(minscale);\
	maxscale=sqrt(maxscale);
	
// Perform LDL^T decomposition of a symmetric positive definite matrix.
// Like Cholesky, but no square roots.  Overwrites lower triangle of matrix.
#define ldltdc( w , diag ) \
	real32 v[2];\
	real32 sum;\
	for (int i = 0; i < 3; i++) {\
		for (int k = 0; k < i; k++) {v[k] = w[i][k] * diag[k];};\
		for (int j = i; j < 3; j++) {\
			sum = w[i][j];\
			for (int k = 0; k < i; k++)\
				sum -= v[k] * w[j][k];\
			if (i == j) {\
				diag[i] = real32(1) / sum;\
			} else {\
				w[j][i] = sum;\
			}\
		}\
	}\



// Solve Ax=B after ldltdc
#define  ldltsl( w , diag , m ) \
	for (int i = 0; i < 3; i++) {\
		sum = m[i];\
		for (int k = 0; k < i; k++)\
			sum -= w[i][k] * m[k];\
		m[i] = sum * diag[i];\
	}\
	for (int i = 2; i >= 0; i--) {\
		sum = 0;\
		for (int k = i + 1; k < 3; k++)\
			sum += w[k][i] * m[k];\
		m[i] -= sum * diag[i];\
	}\

// Rotate a coordinate system to be perpendicular to the given normal
#define   rot_coord_sys(old_u,old_v,new_norm,new_u,new_v) \
	new_u = old_u;\
	new_v = old_v;\
	TVector3 old_norm = old_u ^ old_v;\
	real32 ndot = old_norm * new_norm;\
	if (ndot <= -1.0f) {\
		new_u = -new_u;\
		new_v = -new_v;\
	}else{\
	TVector3 perp_old = new_norm - ndot * old_norm;\
	TVector3 dperp = 1.0f / (1 + ndot) * (old_norm + new_norm);\
	new_u -= dperp * (new_u * perp_old);\
	new_v -= dperp * (new_v * perp_old);};


// Reproject a curvature tensor from the basis spanned by old_u and old_v
// (which are assumed to be unit-length and perpendicular) to the
// new_u, new_v basis.
#define   proj_curv(old_u,old_v,old_ku,old_kuv,old_kv,new_u,new_v,new_ku,new_kuv,new_kv) \
	TVector3 r_new_u, r_new_v;\
	TVector3 new_norm=old_u^old_v;\
	rot_coord_sys(new_u,new_v,new_norm,r_new_u,r_new_v);\
	real32 u1 = r_new_u * old_u;\
	real32 v1 = r_new_u * old_v;\
	real32 u2 = r_new_v * old_u;\
	real32 v2 = r_new_v * old_v;\
	new_ku  = old_ku * u1*u1 + old_kuv * (2.0f  * u1*v1) + old_kv * v1*v1;\
	new_kuv = old_ku * u1*u2 + old_kuv * (u1*v2 + u2*v1) + old_kv * v1*v2;\
	new_kv  = old_ku * u2*u2 + old_kuv * (2.0f  * u2*v2) + old_kv * v2*v2;


// Makes sure that pdir1 and pdir2 are perpendicular to normal
#define   diagonalize_curv(old_u,old_v,ku,kuv,kv,new_norm,pdir1,pdir2,k1,k2) \
	TVector3 r_old_u, r_old_v;\
	rot_coord_sys(old_u,old_v,new_norm,r_old_u,r_old_v);\
	real32 c = 1, s = 0, tt = 0;\
	if (kuv != 0.0f) {\
		real32 h = 0.5f * (kv - ku) / kuv;\
		tt = (h < 0.0f) ?\
			1.0f / (h - sqrt(1.0f + h*h)) :\
			1.0f / (h + sqrt(1.0f + h*h));\
		c = 1.0f / sqrt(1.0f + tt*tt);\
		s = tt * c;\
	}\
	k1 = -(ku - tt * kuv);\
	k2 = -(kv + tt * kuv);\
	if ( CurvaturePMap.Definition )\
		{\
		if (fabs(k1) >= fabs(k2)) {\
			pdir1 = c*r_old_u - s*r_old_v;\
			} else {\
				swapreal32(k1, k2);\
				pdir1 = s*r_old_u + c*r_old_v;\
			}\
		}\
		else\
		{\
		if (k1 >= k2) {\
			pdir1 = c*r_old_u - s*r_old_v;\
			} else {\
				swapreal32(k1, k2);\
				pdir1 = s*r_old_u + c*r_old_v;\
			}\
		}\
	pdir2 = new_norm ^ pdir1;

// Compute Curvatures
#define COMPUTECURVATURES \
	for (int i = 0; i < nf; i++) \
	{\
	ThrowIfNil(cmesh);\
	ThrowIfNil(amesh);\
		cmesh->Principal1s[amesh->fFacets[i].pt1] = amesh->fVertices[amesh->fFacets[i].pt2] - amesh->fVertices[amesh->fFacets[i].pt1];\
		cmesh->Principal1s[amesh->fFacets[i].pt2] = amesh->fVertices[amesh->fFacets[i].pt3] - amesh->fVertices[amesh->fFacets[i].pt2];\
		cmesh->Principal1s[amesh->fFacets[i].pt3] = amesh->fVertices[amesh->fFacets[i].pt1] - amesh->fVertices[amesh->fFacets[i].pt3];\
		cmesh->Principal1s[amesh->fFacets[i].pt1].x*=xfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt1].y*=yfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt1].z*=zfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt2].x*=xfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt2].y*=yfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt2].z*=zfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt3].x*=xfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt3].y*=yfactor;\
		cmesh->Principal1s[amesh->fFacets[i].pt3].z*=zfactor;\
	}\
	for (int i = 0; i < nv; i++) \
	{\
		ThrowIfNil(cmesh);\
		TVector3 normal=cmesh->Normals[i];\
		cmesh->Principal1s[i] =cmesh->Principal1s[i] ^ normal;\
		cmesh->Principal1s[i].Normalize();\
		cmesh->Principal2s[i] = normal ^ cmesh->Principal1s[i];\
	}\
	for (int i = 0; i < nf; i++) \
	{\
		ThrowIfNil(cmesh);\
		ThrowIfNil(amesh);\
		TVector3 e[3] = { amesh->fVertices[amesh->fFacets[i].pt3] - amesh->fVertices[amesh->fFacets[i].pt2],\
			     amesh->fVertices[amesh->fFacets[i].pt1] - amesh->fVertices[amesh->fFacets[i].pt3],\
			     amesh->fVertices[amesh->fFacets[i].pt2] - amesh->fVertices[amesh->fFacets[i].pt1] };\
			     e[0].x*=xfactor;\
			     e[0].y*=yfactor;\
			     e[0].z*=zfactor;\
			     e[1].x*=xfactor;\
			     e[1].y*=yfactor;\
			     e[1].z*=zfactor;\
			     e[2].x*=xfactor;\
			     e[2].y*=yfactor;\
			     e[2].z*=zfactor;\
		TVector3 t = e[0];\
		t.Normalize();\
		TVector3 n = e[0] ^ e[1];\
		TVector3 b = n ^ t;\
		b.Normalize();\
		n.Normalize();\
		real32 m[3] = { 0, 0, 0 };\
		real32 w[3][3] = { {0,0,0}, {0,0,0}, {0,0,0} };\
		for (int j = 0; j < 3; j++) \
		{\
			real32 u = e[j] * t;\
			real32 v = e[j] * b;\
			w[0][0] += u*u;\
			w[0][1] += u*v;\
			w[2][2] += v*v;\
			int32 FacePoint1;\
			if((j+2)%3==0){FacePoint1=amesh->fFacets[i].pt1;};\
			if((j+2)%3==1){FacePoint1=amesh->fFacets[i].pt2;};\
			if((j+2)%3==2){FacePoint1=amesh->fFacets[i].pt3;};\
			int32 FacePoint2;\
			if((j+1)%3==0){FacePoint2=amesh->fFacets[i].pt1;};\
			if((j+1)%3==1){FacePoint2=amesh->fFacets[i].pt2;};\
			if((j+1)%3==2){FacePoint2=amesh->fFacets[i].pt3;};\
			TVector3 normal1=cmesh->Normals[FacePoint1];\
			TVector3 normal2=cmesh->Normals[FacePoint2];\
			TVector3 dn = normal1 - normal2;\
			real32 dnu = dn * t;\
			real32 dnv = dn * b;\
			m[0] += dnu*u;\
			m[1] += dnu*v + dnv*u;\
			m[2] += dnv*v;\
		}\
		w[1][1] = w[0][0] + w[2][2];\
		w[1][2] = w[0][1];\
		real32 diag[3]={0,0,0};\
		ldltdc( w , diag );\
		ldltsl(w , diag , m );\
		for (int j = 0; j < 3; j++) \
		{\
			int vj;\
			if(j==0){vj = amesh->fFacets[i].pt1;};\
			if(j==1){vj = amesh->fFacets[i].pt2;};\
			if(j==2){vj = amesh->fFacets[i].pt3;};\
			real32 c1, c12, c2;\
			proj_curv(t, b, m[0], m[1], m[2],\
				  cmesh->Principal1s[vj], cmesh->Principal2s[vj], c1, c12, c2);\
			real32 wt = cmesh->CornerAreas[i][j] / cmesh->PointAreas[vj];\
			cmesh->k1s[vj]  += wt * c1;\
			cmesh->k12s[vj] += wt * c12;\
			cmesh->k2s[vj]  += wt * c2;\
		};\
	};\
	int i;\
	for ( i = 0; i < nv; i++)\
	{\
	ThrowIfNil(cmesh);\
	TVector3 normal=cmesh->Normals[i];\
	diagonalize_curv(cmesh->Principal1s[i], cmesh->Principal2s[i],\
			 		 cmesh->k1s[i], cmesh->k12s[i], cmesh->k2s[i],\
			 		 normal, cmesh->Principal1s[i], cmesh->Principal2s[i],\
			  		 cmesh->k1s[i],  cmesh->k2s[i]);\
	};

// Compute per-vertex point areas
#define COMPUTEPOINTAREAS \
	for (int i = 0; i < nf; i++) {\
		if(MCVerify(cmesh) && MCVerify(amesh))\
		{\
		ThrowIfNil(cmesh);\
		ThrowIfNil(amesh);\
		TVector3 e[3] = { amesh->fVertices[amesh->fFacets[i].pt3] - amesh->fVertices[amesh->fFacets[i].pt2],\
			     amesh->fVertices[amesh->fFacets[i].pt1] - amesh->fVertices[amesh->fFacets[i].pt3],\
			     amesh->fVertices[amesh->fFacets[i].pt2] - amesh->fVertices[amesh->fFacets[i].pt1] };\
			     e[0].x*=xfactor;\
			     e[0].y*=yfactor;\
			     e[0].z*=zfactor;\
			     e[1].x*=xfactor;\
			     e[1].y*=yfactor;\
			     e[1].z*=zfactor;\
			     e[2].x*=xfactor;\
			     e[2].y*=yfactor;\
			     e[2].z*=zfactor;\
		real32 area = 0.5f * (e[0] ^ e[1]).GetMagnitude();\
		real32 l2[3] = { (e[0]).GetMagnitudeSquared(), (e[1]).GetMagnitudeSquared(), (e[2]).GetMagnitudeSquared() };\
		real32 ew[3] = { l2[0] * (l2[1] + l2[2] - l2[0]),l2[1] * (l2[2] + l2[0] - l2[1]),l2[2] * (l2[0] + l2[1] - l2[2]) };\
		if (ew[0] <= 0.0f) {\
			cmesh->CornerAreas[i][1] = -0.25f * l2[2] * area /(e[0] * e[2]);\
			cmesh->CornerAreas[i][2] = -0.25f * l2[1] * area /(e[0] * e[1]);\
			cmesh->CornerAreas[i][0] = area - cmesh->CornerAreas[i][1] - cmesh->CornerAreas[i][2];\
		} else if (ew[1] <= 0.0f) {\
			cmesh->CornerAreas[i][2] = -0.25f * l2[0] * area /(e[1] * e[0]);\
			cmesh->CornerAreas[i][0] = -0.25f * l2[2] * area /(e[1] * e[2]);\
			cmesh->CornerAreas[i][1] = area - cmesh->CornerAreas[i][2] - cmesh->CornerAreas[i][0];\
		} else if (ew[2] <= 0.0f) {\
			cmesh->CornerAreas[i][0] = -0.25f * l2[1] * area /(e[2] * e[1]);\
			cmesh->CornerAreas[i][1] = -0.25f * l2[0] * area /(e[2] * e[0]);\
			cmesh->CornerAreas[i][2] = area - cmesh->CornerAreas[i][0] - cmesh->CornerAreas[i][1];\
		} else {\
			float ewscale = 0.5f * area / (ew[0] + ew[1] + ew[2]);\
			for (int j = 0; j < 3; j++)\
				cmesh->CornerAreas[i][j] = ewscale * (ew[(j+1)%3] + ew[(j+2)%3]);\
		}\
		cmesh->PointAreas[amesh->fFacets[i].pt1] += cmesh->CornerAreas[i][0];\
		cmesh->PointAreas[amesh->fFacets[i].pt2] += cmesh->CornerAreas[i][1];\
		cmesh->PointAreas[amesh->fFacets[i].pt3] += cmesh->CornerAreas[i][2];\
		};\
	}
	
