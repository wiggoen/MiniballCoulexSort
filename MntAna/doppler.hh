#ifndef doppler_hh
#define doppler_hh

#include "TRandom.h"
#include "TRandom3.h"
#include "TCutG.h"
#include "TGaxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TMath.h"
#include "TObject.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>
using namespace std;

// Experimental definitions
#ifndef ExpDefs_hh
# include "ExpDefs.hh"
#endif

class doppler : public TObject {

	public:

	static Int_t	Cut(Double_t PEn, Double_t anno, Int_t quad);
	static Int_t	Cut_2p(Double_t PEn1, Double_t anno1, Int_t quad1,
							Double_t PEn2, Double_t anno2, Int_t quad2);
	static bool	CutG_en2hit( double BEn, double TEn );
	static Double_t	GetTarDist();
	static Double_t	GetPTh(Double_t anno);
	static Double_t	GetPPhi(Int_t quad, Int_t seg, float offset=242.6);//TODO Check this
	static Double_t	GetTTh(Double_t Banno, Double_t BEn);
	static Double_t	GetBTh(Double_t Tanno);
	static Double_t	GetQPhi(Int_t quad, Int_t seg);
	static Double_t	GetTEn(Double_t BEn, Double_t Banno);
	static Double_t	GetBEn(Double_t TEn, Double_t Tanno);
	static Double_t	GetELoss(Double_t Ei, Double_t dist, Int_t opt, string combo);
	static Double_t	GammaAng(Double_t PTh, Double_t PPhi, Double_t GTh, Double_t GPhi);
	static Double_t	DC(Double_t PEn, Double_t PTh, Double_t PPhi, Double_t GTh, Double_t GPhi, Double_t A);
	static Double_t	DC_elec(double een, double PEn, double PTh, double PPhi, double GTh, double GPhi, double A);
	static bool stoppingpowers( Int_t Zp, Int_t Zt, Double_t Ap, Double_t At, string opt );
	static string convertInt( int number );
	static string convertFloat( float number );
	
	private:
	static TRandom3 rand;
	static double BTELoss_pars[6];
	static double TTELoss_pars[6];
	static double BSELoss_pars[6];
	static double TSELoss_pars[6];
	static string gElName[110];
	static double SP_function( double *x, double *par );
	static double gates[64][3];
	
	ClassDef(doppler,1);

};
#endif
