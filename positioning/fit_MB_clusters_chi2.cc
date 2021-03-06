// Simultaneous fit of all segment angles in a MB cluster

#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "Fit/Chi2FCN.h"
#include "TMinuit.h"
#include "Math/Minimizer.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TList.h"
#include "TMath.h"
#include "Math/WrappedMultiTF1.h"
#include "Math/SpecFunc.h"
#include "Math/SpecFuncMathMore.h"
#include "HFitInterface.h"
#include "TCanvas.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TVector3.h"
#include "TAxis.h"
#include "TGraph.h"
#include "TROOT.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

#define ROOTTHREEOVER2 0.8660254
#define DIST_CORE_CORNER 34

#define NCLU 2
#define NSEG 18
#define IPARS 5 	// per data set: r, theta, phi, alpha, beta
#define NPARS 8 	// global fit: 2*r + theta + 2*phi + 2*alpha + beta

// data holders
double energies[8][NSEG];
double en_error[8][NSEG];
int clu_num[8];

// definition of shared parameters
int ipar[NCLU*NSEG][IPARS];

double par0[NPARS];

// initial parameters and names
string parname[NPARS] = { "r_0", "phi_0", "alpha_0", "r_1", "phi_1", "alpha_1", "theta", "beta" };

// Ordering of cluster numbers for setup
void setup_clusters() {
	
	// Set the cluster numbers
	clu_num[0] = 17;	// = 13
	clu_num[1] = 12;	// = 16
	clu_num[2] = 16;	// = 12
	clu_num[3] = 13;	// = 17
	clu_num[4] = 22;	// = 14
	clu_num[5] = 18;	// = 23
	clu_num[6] = 14;	// = 22
	clu_num[7] = 23;	// = 18
	
	return;
	
}

// Find the index of the cluster
int lookup_clu( int myclu ) {
	
	for( int i = 0; i < 8; i++ ) {
		
		if( myclu == clu_num[i] ) return(i);
		
	}
	
	return(-1);

}

// String convert function
string convertInt( int number ) {
	
	stringstream ss;
	ss << number;
	return ss.str();
	
}

///////////////////////////
// Fit stuff starts here //
///////////////////////////

// Make initial starting values from the frame readings
void initialise_parameters( int *myclu ) {
	
	double r[8] = { 153.1, 131.4, 129.3, 127.7, 127.6, 132.8, 131.8, 130.1 };
	double theta[8] = { 142.5, 57.7, 57.7, 142.5, 145.4, 55.9, 145.4, 55.9 };
	double phi[8] = { 127.0, 52.5, 133.5, 57.0, 232.9, 222.5, 297.5, 228.5 };
	double alpha[8] = { 309.5, 173.0, 122.0, 118.0, 50.3, 4.0, 148.6, 350.0 };
	double beta = 0.0656; // for 2.85 MeV/u, what is it now?
	
	par0[0] = r[lookup_clu(myclu[0])];
	par0[1] = phi[lookup_clu(myclu[0])];
	par0[2] = alpha[lookup_clu(myclu[0])];

	par0[3] = r[lookup_clu(myclu[1])];
	par0[4] = phi[lookup_clu(myclu[1])];
	par0[5] = alpha[lookup_clu(myclu[1])];
	
	par0[6] = theta[lookup_clu(myclu[0])];
	par0[7] = beta;
	
	for( int i = 0; i < NCLU; i++ )
		cout << "Cluster " << myclu[i] << " has index " << lookup_clu(myclu[i]) << endl;
	
	return;
	
}

// Make lists of parameters for each segment
void assign_parameters() {

	for( int i = 0; i < NCLU; i++ ) {
		
		for( int j = 0; j < NSEG; j++ ) {
			
			ipar[i*NSEG+j][0] = i*3 + 0;  // r
			ipar[i*NSEG+j][1] = i*3 + 1;  // phi
			ipar[i*NSEG+j][2] = i*3 + 2;  // alpha
			ipar[i*NSEG+j][3] = NCLU*3 + (int)(i/2);  // theta (pair wise)
			ipar[i*NSEG+j][4] = NPARS - 1;  // beta (global)
			
		}
	
	}
	
	return;

}

// Get data from file
void read_energies() {
	
	FILE *fp_dn;
	Double_t energy_dn, error_dn;
	Int_t myclu_dn, seg_dn, status_dn, pos_dn;
	char line_dn[1024], det_dn;
	
	// Open file
	fp_dn = fopen( "analyse_22Ne_E440.dat", "r" );
	if( !fp_dn ) {
		fprintf( stderr, "Unable to read analyse_22Ne_E440.dat\n");
		exit(-1);
	}
	
	// Initialise
	for( int i = 0; i < NCLU; i++ ) {

		for( int j = 0; j < NSEG; j++ ) {
		
			energies[i][j] = -999.;
			en_error[i][j] =  999.;
			
		}
		
	}
	
	// Read file
	while(1) {
		
		// Read line
		if( fgets( line_dn, sizeof(line_dn) - 1, fp_dn ) == NULL) break;
		
		// Get values
		status_dn = sscanf( line_dn, "%d%c%d %lf %lf", &myclu_dn, &det_dn, &seg_dn, &energy_dn, &error_dn );
		if( status_dn != 5 ) continue;
		
		// Store it
		pos_dn = (det_dn - 'A') * 6 + (seg_dn - 1);
		if( error_dn < 1e-6 ) continue; // skip zero error points
		energies[lookup_clu(myclu_dn)][pos_dn] = energy_dn;
		en_error[lookup_clu(myclu_dn)][pos_dn] = error_dn;
		
	}
	
	// Close file
	fclose(fp_dn);

	return;

}

double seg_fit_function( double *x, double *par ) {
	
	// Get parameters from list
	double r = par[0];
	double phi = par[1];
	double alpha = par[2];
	double theta = par[3];
	double beta = par[4];
	int segNo = (int)x[0];
	
	// Angles and geometry
	TVector3 seg_offset[18], det_offset[3];
	Double_t R = DIST_CORE_CORNER * 0.6; // average distance from centre of capsule to center of segment
	
	// Offset from centre of cluster to centre of each detector
	det_offset[0].SetXYZ(DIST_CORE_CORNER,      0,                                 r);
	det_offset[1].SetXYZ(-DIST_CORE_CORNER / 2, -DIST_CORE_CORNER * ROOTTHREEOVER2, r);
	det_offset[2].SetXYZ(-DIST_CORE_CORNER / 2, DIST_CORE_CORNER * ROOTTHREEOVER2,  r);
	
	// Offset from centre of a detector to centre of a segment
	seg_offset[0].SetXYZ(R / 2,  -R * ROOTTHREEOVER2,  0.0);
	seg_offset[1].SetXYZ(-R / 2, -R * ROOTTHREEOVER2,  0.0);
	seg_offset[2].SetXYZ(R,       0.0,                  0.0);
	seg_offset[3].SetXYZ(R / 2,   R * ROOTTHREEOVER2,   0.0);
	seg_offset[4].SetXYZ(-R,      0.0,                  0.0);
	seg_offset[5].SetXYZ(-R / 2,  R * ROOTTHREEOVER2,   0.0);
	
	seg_offset[6].SetXYZ(-R,      0.0,                  0.0);
	seg_offset[7].SetXYZ(-R / 2,  R * ROOTTHREEOVER2,   0.0);
	seg_offset[8].SetXYZ(-R / 2,  -R * ROOTTHREEOVER2,  0.0);
	seg_offset[9].SetXYZ(R / 2,   -R * ROOTTHREEOVER2,  0.0);
	seg_offset[10].SetXYZ(R / 2,  R * ROOTTHREEOVER2,   0.0);
	seg_offset[11].SetXYZ(R,      0.0,                  0.0);
	
	seg_offset[12].SetXYZ(R / 2,   R * ROOTTHREEOVER2,  0.0);
	seg_offset[13].SetXYZ(R,       0.0,                 0.0);
	seg_offset[14].SetXYZ(-R / 2,  R * ROOTTHREEOVER2,  0.0);
	seg_offset[15].SetXYZ(-R,      0.0,                 0.0);
	seg_offset[16].SetXYZ(R / 2,   -R * ROOTTHREEOVER2, 0.0);
	seg_offset[17].SetXYZ(-R / 2,  -R * ROOTTHREEOVER2, 0.0);
	
	// Add the segment offsets to the detector offsets, so now
	// seg_offset has the offset from the centre of the cluster
	for( int i = 0; i < NSEG; i++ )
		seg_offset[i] += det_offset[i/6];
	
	// Sort out phi
	if( phi > 180 ) {
		theta = 180. - theta;
	}
	
	// Transform
	TVector3 trans;
	
	trans.SetXYZ(1,0,0);
	trans.RotateY( -phi * TMath::DegToRad() );
	trans.RotateX( -theta * TMath::DegToRad() );
	theta = trans.Theta() * TMath::RadToDeg();
	phi   = trans.Phi() * TMath::RadToDeg();
	
	// Sort out alpha offset
	TVector3 va, vd, vn, ex(1,0,0);
	
	vd.SetXYZ(0,0,1);
	va.SetXYZ(1,0,0);
	vd.RotateY( theta * TMath::DegToRad() );
	vd.RotateZ( phi * TMath::DegToRad() );
	va.RotateY( theta * TMath::DegToRad() );
	va.RotateZ( phi * TMath::DegToRad() );
	
	vn = ex - vd * (ex * vd);
	vn.SetMag(1.);
	
	Double_t alpha_offset;
	alpha_offset = va.Angle(vn);
	if( vn * (va.Cross(vd)) < 0 ) alpha_offset = -alpha_offset;
	
	// Rotate segments
	for( int i = 0; i < NSEG; i++ ) {
		
		seg_offset[i].RotateZ( -alpha * TMath::DegToRad() - alpha_offset );
		seg_offset[i].RotateY( theta * TMath::DegToRad() );
		seg_offset[i].RotateZ( phi * TMath::DegToRad() );
		
	}
	
	// Calculate the energies
	double dc = ( 1 - beta * TMath::Cos( seg_offset[segNo].Theta() ) ) / TMath::Sqrt( 1.0 - beta*beta );
	
	return 439.99 / dc;

}


struct GlobalChi2 {
	
	GlobalChi2( ROOT::Math::IMultiGenFunction & f0,  ROOT::Math::IMultiGenFunction & f1,
			    ROOT::Math::IMultiGenFunction & f2,  ROOT::Math::IMultiGenFunction & f3,
			    ROOT::Math::IMultiGenFunction & f4,  ROOT::Math::IMultiGenFunction & f5,
			    ROOT::Math::IMultiGenFunction & f6,  ROOT::Math::IMultiGenFunction & f7,
			    ROOT::Math::IMultiGenFunction & f8,  ROOT::Math::IMultiGenFunction & f9,
			    ROOT::Math::IMultiGenFunction & f10, ROOT::Math::IMultiGenFunction & f11,
			    ROOT::Math::IMultiGenFunction & f12, ROOT::Math::IMultiGenFunction & f13,
			    ROOT::Math::IMultiGenFunction & f14, ROOT::Math::IMultiGenFunction & f15,
			    ROOT::Math::IMultiGenFunction & f16, ROOT::Math::IMultiGenFunction & f17,
			    ROOT::Math::IMultiGenFunction & f18, ROOT::Math::IMultiGenFunction & f19,
			    ROOT::Math::IMultiGenFunction & f20, ROOT::Math::IMultiGenFunction & f21,
			    ROOT::Math::IMultiGenFunction & f22, ROOT::Math::IMultiGenFunction & f23,
			    ROOT::Math::IMultiGenFunction & f24, ROOT::Math::IMultiGenFunction & f25,
			    ROOT::Math::IMultiGenFunction & f26, ROOT::Math::IMultiGenFunction & f27,
			    ROOT::Math::IMultiGenFunction & f28, ROOT::Math::IMultiGenFunction & f29,
			    ROOT::Math::IMultiGenFunction & f30, ROOT::Math::IMultiGenFunction & f31,
			    ROOT::Math::IMultiGenFunction & f32, ROOT::Math::IMultiGenFunction & f33,
			    ROOT::Math::IMultiGenFunction & f34, ROOT::Math::IMultiGenFunction & f35 ) :
	fChi2_0(&f0),   fChi2_1(&f1),   fChi2_2(&f2),   fChi2_3(&f3),   fChi2_4(&f4),   fChi2_5(&f5),
	fChi2_6(&f6),   fChi2_7(&f7),   fChi2_8(&f8),   fChi2_9(&f9),   fChi2_10(&f10), fChi2_11(&f11),
	fChi2_12(&f12), fChi2_13(&f13), fChi2_14(&f14), fChi2_15(&f15), fChi2_16(&f16), fChi2_17(&f17),
	fChi2_18(&f18), fChi2_19(&f19), fChi2_20(&f20), fChi2_21(&f21), fChi2_22(&f22), fChi2_23(&f23),
	fChi2_24(&f24), fChi2_25(&f25), fChi2_26(&f26), fChi2_27(&f27), fChi2_28(&f28), fChi2_29(&f29),
	fChi2_30(&f30), fChi2_31(&f31), fChi2_32(&f32), fChi2_33(&f33), fChi2_34(&f34), fChi2_35(&f35) {}
	
	
	double operator() (const double *par) const {
		
		double p[NCLU*NSEG][IPARS];
		
		for ( int i = 0; i < NCLU*NSEG; i++ ) {
			
			for ( int j = 0; j < IPARS; j++ ) {
				
				p[i][j]  = par[ ipar[i][j] ];
				
			}
			
		}
		
		return (*fChi2_0)(p[0]) + (*fChi2_1)(p[1]) + (*fChi2_2)(p[2]) + (*fChi2_3)(p[4]) + (*fChi2_4)(p[4])
			+ (*fChi2_5)(p[5]) + (*fChi2_6)(p[6]) + (*fChi2_7)(p[7]) + (*fChi2_8)(p[8]) + (*fChi2_9)(p[9])
			+ (*fChi2_10)(p[10]) + (*fChi2_11)(p[11]) + (*fChi2_12)(p[12]) + (*fChi2_13)(p[13])
			+ (*fChi2_14)(p[14]) + (*fChi2_15)(p[15]) + (*fChi2_16)(p[16]) + (*fChi2_17)(p[17])
			+ (*fChi2_18)(p[18]) + (*fChi2_19)(p[19]) + (*fChi2_20)(p[20]) + (*fChi2_21)(p[21])
			+ (*fChi2_22)(p[22]) + (*fChi2_23)(p[23]) + (*fChi2_24)(p[24]) + (*fChi2_25)(p[25])
			+ (*fChi2_26)(p[26]) + (*fChi2_27)(p[27]) + (*fChi2_28)(p[28]) + (*fChi2_29)(p[29])
			+ (*fChi2_30)(p[30]) + (*fChi2_31)(p[31]) + (*fChi2_32)(p[32]) + (*fChi2_33)(p[33])
			+ (*fChi2_34)(p[34]) + (*fChi2_35)(p[35]);
		
	}
	
	const ROOT::Math::IMultiGenFunction *fChi2_0, *fChi2_1, *fChi2_2, *fChi2_3, *fChi2_4, *fChi2_5,
		*fChi2_6, *fChi2_7, *fChi2_8, *fChi2_9, *fChi2_10, *fChi2_11, *fChi2_12, *fChi2_13, *fChi2_14,
		*fChi2_15, *fChi2_16, *fChi2_17, *fChi2_18, *fChi2_19, *fChi2_20, *fChi2_21, *fChi2_22, *fChi2_23,
		*fChi2_24, *fChi2_25, *fChi2_26, *fChi2_27, *fChi2_28, *fChi2_29, *fChi2_30, *fChi2_31, *fChi2_32,
		*fChi2_33, *fChi2_34, *fChi2_35;
	
};

void fit_MB_clusters_chi2( int clu0, int clu1 ) {
	
	// Some setup
	setup_clusters();
	int myclu[NCLU];
	myclu[0] = clu0;
	myclu[1] = clu1;
	
	// Initialise parameters
	assign_parameters();
	initialise_parameters( myclu );

	// Read in experimental data from file
	read_energies();
	
	// Fit functions for angles
	TF1 *fAng[NCLU*NSEG];
	string name;
	for ( int i = 0; i < NCLU*NSEG; i++ ) {
		
		name = "fAng_" + convertInt(i+1);
		fAng[i] = new TF1( name.c_str(), seg_fit_function, -0.5, NSEG+0.5, IPARS );
		
	}
	
	//////////////////////////
	// Perform a global fit //
	//////////////////////////
	
	// First make individual fit wrappers
	ROOT::Math::WrappedMultiTF1 wClu_0( *fAng[0], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_1( *fAng[1], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_2( *fAng[2], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_3( *fAng[3], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_4( *fAng[4], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_5( *fAng[5], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_6( *fAng[6], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_7( *fAng[7], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_8( *fAng[8], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_9( *fAng[9], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_10( *fAng[10], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_11( *fAng[11], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_12( *fAng[12], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_13( *fAng[13], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_14( *fAng[14], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_15( *fAng[15], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_16( *fAng[16], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_17( *fAng[17], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_18( *fAng[18], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_19( *fAng[19], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_20( *fAng[20], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_21( *fAng[21], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_22( *fAng[22], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_23( *fAng[23], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_24( *fAng[24], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_25( *fAng[25], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_26( *fAng[26], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_27( *fAng[27], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_28( *fAng[28], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_29( *fAng[29], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_30( *fAng[30], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_31( *fAng[31], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_32( *fAng[32], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_33( *fAng[33], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_34( *fAng[34], 1 );
	ROOT::Math::WrappedMultiTF1 wClu_35( *fAng[35], 1 );
	
	// Data initialisers and filling
	ROOT::Fit::DataOptions opt;
	ROOT::Fit::BinData data[NCLU*NSEG];

	for( int i = 0; i < NCLU; i++ ) {

		for( int j = 0; j < NSEG; j++ ) {
			
			data[i*NSEG+j] = ROOT::Fit::BinData( opt, 1, 1 );
			data[i*NSEG+j].Add( (double)j, energies[lookup_clu(myclu[i])][j],
							   en_error[lookup_clu(myclu[i])][j] );

		}
		
	}

	// Get data size
	int data_size = 0;
	for( int i = 0; i < NCLU*NSEG; i++ ) data_size += data[i].Size();
	
	// Define chi2 functions
	ROOT::Fit::Chi2Function chi2_0( data[0], wClu_0 );
	ROOT::Fit::Chi2Function chi2_1( data[1], wClu_1 );
	ROOT::Fit::Chi2Function chi2_2( data[2], wClu_2 );
	ROOT::Fit::Chi2Function chi2_3( data[3], wClu_3 );
	ROOT::Fit::Chi2Function chi2_4( data[4], wClu_4 );
	ROOT::Fit::Chi2Function chi2_5( data[5], wClu_5 );
	ROOT::Fit::Chi2Function chi2_6( data[6], wClu_6 );
	ROOT::Fit::Chi2Function chi2_7( data[7], wClu_7 );
	ROOT::Fit::Chi2Function chi2_8( data[8], wClu_8 );
	ROOT::Fit::Chi2Function chi2_9( data[9], wClu_9 );
	ROOT::Fit::Chi2Function chi2_10( data[10], wClu_10 );
	ROOT::Fit::Chi2Function chi2_11( data[11], wClu_11 );
	ROOT::Fit::Chi2Function chi2_12( data[12], wClu_12 );
	ROOT::Fit::Chi2Function chi2_13( data[13], wClu_13 );
	ROOT::Fit::Chi2Function chi2_14( data[14], wClu_14 );
	ROOT::Fit::Chi2Function chi2_15( data[15], wClu_15 );
	ROOT::Fit::Chi2Function chi2_16( data[16], wClu_16 );
	ROOT::Fit::Chi2Function chi2_17( data[17], wClu_17 );
	ROOT::Fit::Chi2Function chi2_18( data[18], wClu_18 );
	ROOT::Fit::Chi2Function chi2_19( data[19], wClu_19 );
	ROOT::Fit::Chi2Function chi2_20( data[20], wClu_20 );
	ROOT::Fit::Chi2Function chi2_21( data[21], wClu_21 );
	ROOT::Fit::Chi2Function chi2_22( data[22], wClu_22 );
	ROOT::Fit::Chi2Function chi2_23( data[23], wClu_23 );
	ROOT::Fit::Chi2Function chi2_24( data[24], wClu_24 );
	ROOT::Fit::Chi2Function chi2_25( data[25], wClu_25 );
	ROOT::Fit::Chi2Function chi2_26( data[26], wClu_26 );
	ROOT::Fit::Chi2Function chi2_27( data[27], wClu_27 );
	ROOT::Fit::Chi2Function chi2_28( data[28], wClu_28 );
	ROOT::Fit::Chi2Function chi2_29( data[29], wClu_29 );
	ROOT::Fit::Chi2Function chi2_30( data[30], wClu_30 );
	ROOT::Fit::Chi2Function chi2_31( data[31], wClu_31 );
	ROOT::Fit::Chi2Function chi2_32( data[32], wClu_32 );
	ROOT::Fit::Chi2Function chi2_33( data[33], wClu_33 );
	ROOT::Fit::Chi2Function chi2_34( data[34], wClu_34 );
	ROOT::Fit::Chi2Function chi2_35( data[35], wClu_35 );

	GlobalChi2 globalChi2( chi2_0, chi2_1, chi2_2, chi2_3, chi2_4, chi2_5, chi2_6, chi2_7, chi2_8,
						  chi2_9, chi2_10, chi2_11, chi2_12, chi2_13, chi2_14, chi2_15, chi2_16,
						  chi2_17, chi2_18, chi2_19, chi2_20, chi2_21, chi2_22, chi2_23, chi2_24,
						  chi2_25, chi2_26, chi2_27, chi2_28, chi2_29, chi2_30, chi2_31, chi2_32,
						  chi2_33, chi2_34, chi2_35 );

	ROOT::Fit::Fitter fitter;

	// create before the parameter settings in order to fix or set range on them
	fitter.Config().SetParamsSettings( NPARS, par0 );

	// set parameter names
	for ( int i = 0; i < NPARS; i++ )
		fitter.Config().ParSettings(i).SetName( parname[i].c_str() );

	// set parameter limits
	for( int i = 0; i < NCLU; i++ ) {
		
		// r values must be positive
		fitter.Config().ParSettings(ipar[i*NSEG+0][0]).SetLowerLimit( 0.0 );
		
		// phi values <360
		fitter.Config().ParSettings(ipar[i*NSEG+0][1]).SetLowerLimit(   0.0 );
		fitter.Config().ParSettings(ipar[i*NSEG+0][1]).SetUpperLimit( 360.0 );
		
		// alpha values <360
		fitter.Config().ParSettings(ipar[i*NSEG+0][2]).SetLowerLimit(   0.0 );
		fitter.Config().ParSettings(ipar[i*NSEG+0][2]).SetUpperLimit( 360.0 );
		
		// theta values <180
		fitter.Config().ParSettings(ipar[i*NSEG+0][3]).SetLowerLimit(   0.0 );
		fitter.Config().ParSettings(ipar[i*NSEG+0][3]).SetUpperLimit( 180.0 );
		
	}
	

	// fix parameters by uncommenting and/or adding new lines
	fitter.Config().ParSettings(7).Fix();	// beta = 0.0656

	//fitter.Config().SetMinimizer( "Minuit2", "migrad" ); // Minuit2 and Migrad as defaults (best!) - alternative minimiser is simplex
	fitter.Config().SetMinimizer( "GSLMultiFit" );
	fitter.Config().MinimizerOptions().SetMaxIterations(99999);
	fitter.Config().MinimizerOptions().SetMaxFunctionCalls(99999);
	fitter.Config().MinimizerOptions().SetPrintLevel(1);
	
	//fitter.Config().MinimizerOptions().SetPrecision(1.0e-9);
	fitter.Config().MinimizerOptions().SetTolerance(2.0e-5);
	cout << "Minimizer precision = " << fitter.Config().MinimizerOptions().Precision() << endl;
	cout << "Minimizer tolerance = " << fitter.Config().MinimizerOptions().Tolerance() << endl;

	// fit FCN function directly
	fitter.FitFCN( NPARS, globalChi2, par0, data_size, true );
	
	// Make comparison of values
	double segNo[1];
	double tmp_par[IPARS];
	double calc_en, exp_en, err_en;
	
	cout << "Clu-Seg\t     Exp.\t     Err.\t     Calc.\t  Sigma" << endl;
	for( int i = 0; i < NCLU; i++ ) {
		
		for( int j = 0; j < NSEG; j++ ) {
			
			cout << myclu[i] << "-" << j;
			
			segNo[0] = j;
			for( int k = 0; k < IPARS; k++ )
				tmp_par[k] = par0[ ipar[i*NSEG+j][k] ];
			
			exp_en = data[i*NSEG+j].Value(0);
			err_en = data[i*NSEG+j].Error(0);
			calc_en = seg_fit_function( segNo, tmp_par );
			
			cout << "\t" << setfill(' ') << setw(9) << exp_en;
			cout << "\t" << setfill(' ') << setw(9) << err_en;
			cout << "\t" << setfill(' ') << setw(9) << calc_en;
			cout << "\t" << setfill(' ') << setw(7) << (exp_en - calc_en) / err_en;
			cout << endl;
			
		}
		
	}
	
	// Print result to screen/file
	ROOT::Fit::FitResult result = fitter.Result();
	ofstream fitfile;
	fitfile.open( "fitresult.txt", ios::out );
	result.Print( std::cout );
	result.Print( fitfile );
	result.PrintCovMatrix( fitfile );

	return;

}
