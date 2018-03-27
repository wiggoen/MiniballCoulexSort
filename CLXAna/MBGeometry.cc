#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <TVector3.h>
#include <TAxis.h>
#include <TGraph.h>
#include <TMath.h>
#include <TROOT.h>

#include "MBGeometry.hh"

// Constructor and destructor
MBGeometry::MBGeometry(){

	//cout << "constructor" << endl;

}

MBGeometry::~MBGeometry(){

	//cout << "destructor" << endl;

}

void MBGeometry::SetCluTheta( double user_theta ) {

	/// Set the theta angle measured from the frame

	// Set the user value
	theta = user_theta;

	return;

}

void MBGeometry::SetCluPhi( double user_phi ) {

	/// Set the phi angle measured from the frame
	
	// Set the user value
	phi = user_phi;

	return;

}

void MBGeometry::SetCluAlpha( double user_alpha ) {

	/// Set the alpha angle measured from the frame
	
	// Set the user value
	alpha = user_alpha;

	return;

}

void MBGeometry::SetCluR( double user_r ) {

	/// Set the distance between the target and face of the cluster
	
	// Set the user value (mm)
	r = user_r;

	return;

}

double MBGeometry::GetSegTheta( int core, int seg ) {

	/// Get the theta angle of a segment with respect to the beam
	
	// Calculate the segment number
	int segNo = core*6 + seg;

	// Return theta from segment offset (old)
	//return seg_offset[segNo].Theta() * TMath::RadToDeg();

	// New method of returning theta in true coordinate system
	return TrueTheta( seg_offset[segNo] );

}

double MBGeometry::GetSegPhi( int core, int seg ) {

	/// Get the phi angle of a segment with respect to the beam
	
	// Calculate the segment number
	int segNo = core*6 + seg;

	// Return phi from segment offset (old)
	//return seg_offset[segNo].Phi() * TMath::RadToDeg();

	// New method of returning phi in true coordinate system
	return TruePhi( seg_offset[segNo] );

}

double MBGeometry::GetCoreTheta( int core ) {

	/// Get the theta angle of the core with respect to the beam
	
	// Return theta from segment offset
	return TrueTheta( det_offset[core] );

}

// Get a core phi
double MBGeometry::GetCorePhi( int core ) {

	/// Get the phi angle of the core with respect to the beam
	
	// Return phi from segment offset
	return TruePhi( det_offset[core] );

}

void MBGeometry::SetupCluster( double user_theta, double user_phi, double user_alpha, double user_r ) {

	/// Setup the cluster with coordinate values

	// Set the user value
	theta = user_theta * TMath::DegToRad();
	phi = user_phi * TMath::DegToRad();
	alpha = user_alpha * TMath::DegToRad();
	r = user_r;
	if( phi > TMath::Pi() ) theta = TMath::Pi() - theta;

	SetupCluster();

	return;

}

void MBGeometry::SetupCluster() {

	/// Setup cluster main routine

	Double_t R = DIST_CORE_CORNER * 0.5; // average distance from centre of capsule to center of segment

	// Offset of cluster
	clu_offset.SetXYZ(0, r, 0);

	// Offset from centre of cluster to centre of each detector
	det_offset[0].SetXYZ( 0,					   r, -DIST_CORE_CORNER);
	det_offset[1].SetXYZ(-DIST_CORE_CORNER * ROOTTHREEOVER2, r,  DIST_CORE_CORNER / 2);
	det_offset[2].SetXYZ( DIST_CORE_CORNER * ROOTTHREEOVER2, r,  DIST_CORE_CORNER / 2);
	
	// Offset from centre of a detector to centre of a segment
	seg_offset[0 ].SetXYZ(-R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[1 ].SetXYZ(-R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[2 ].SetXYZ( 0.0,                0.0, -R     );
	seg_offset[3 ].SetXYZ( R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[4 ].SetXYZ( 0.0,                0.0,  R     );
	seg_offset[5 ].SetXYZ( R * ROOTTHREEOVER2, 0.0,  R / 2 );

	seg_offset[6 ].SetXYZ( 0.0,                0.0,  R     );
	seg_offset[7 ].SetXYZ( R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[8 ].SetXYZ(-R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[9 ].SetXYZ(-R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[10].SetXYZ( R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[11].SetXYZ( 0.0,                0.0, -R     );

	seg_offset[12].SetXYZ( R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[13].SetXYZ( 0.0,                0.0, -R     );
	seg_offset[14].SetXYZ( R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[15].SetXYZ( 0.0,                0.0,  R     );
	seg_offset[16].SetXYZ(-R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[17].SetXYZ(-R * ROOTTHREEOVER2, 0.0,  R / 2 );
   
   	// Add the segment offsets to the detector offsets, so now
   	// seg_offset has the offset from the centre of the cluster
	for( int i = 0; i < 3; i++ )
		for( int j = 0; j < 6; j++ )
			seg_offset[i * 6 + j] += det_offset[i];

	// Offsets
	double myalpha, mytheta, myphi;
	if( phi < TMath::Pi() ) {
		myalpha = alpha;
		myphi = TMath::Pi() / 2. + phi;
		mytheta = TMath::Pi() / 2. + theta;
	}

	else {
		myalpha = alpha - TMath::Pi();
		myphi = TMath::Pi() / 2. + phi;
		mytheta = TMath::Pi() / 2. + theta;
	}

	// Rotate cluster to appropriate angle
	clu_offset.RotateY(myalpha);
	clu_offset.RotateX(myphi);
	clu_offset.RotateZ(mytheta);
		
	// Rotate cores to appropriate angle
	for( UInt_t i = 0; i < 3; i++ ) {
		det_offset[i].RotateY(myalpha);
		det_offset[i].RotateX(myphi);
		det_offset[i].RotateZ(mytheta);
	}

	// Rotate segments to appropriate angle
	for( UInt_t i = 0; i < 18; i++ ) {
		seg_offset[i].RotateY(myalpha);
		seg_offset[i].RotateX(myphi);
		seg_offset[i].RotateZ(mytheta);
	}
 
 	return;
 	
}

