#include "TreeBuilder.hh"

#define PBINS 500
#define PRANGE 500
#define PMIN -1.0*PRANGE/PBINS
#define PMAX PRANGE+PMIN

#define GBINS 4000
#define GRANGE 4000
#define GMIN -1.0*GRANGE/GBINS
#define GMAX GRANGE+GMIN
#define GAMMA_ARRAY 250

#define ELBINS 2000
#define ELRANGE 1000
#define ELMIN -1.0*ELRANGE/ELBINS
#define ELMAX ELRANGE+ELMIN
#define ELECTRON_ARRAY 250

ClassImp(BuiltEvent)
ClassImp(AdcData)
ClassImp(DgfData)

int main(int argc, char* argv[]) {

	vector<char*> InputFiles;
	string OutputFile;
	string CalibrationFile;
	bool NoPad = false;
	bool OnlyFBCD = false;
	bool VetoBCD = false;
	bool CheckParPos = false;
	bool Singles = false;
	bool crex = false;
	bool cdpad = false;
	bool ionch = false;
	bool Spede = false;
	bool verbose = false;
	CommandLineInterface* interface = new CommandLineInterface();

	interface->Add("-i", "inputfiles", &InputFiles );
	interface->Add("-o", "outputfile", &OutputFile );
	interface->Add("-np", "NoPadTriggering", &NoPad );
	interface->Add("-fcd", "OnlyFBCDTrig", &OnlyFBCD );
	interface->Add("-vbcd", "VetoBCD", &VetoBCD );
	interface->Add("-chpart", "CheckParPos", &CheckParPos );
	interface->Add("-c", "calibrationfile", &CalibrationFile );
	interface->Add("-s", "singles", &Singles );
	interface->Add("-crex", "CREX", &crex );
	interface->Add("-spede", "SPEDE", &Spede );
	interface->Add("-cdpad", "CD-PAD", &cdpad );
	interface->Add("-ionch", "IonChamber", &ionch );
	interface->Add("-vl", "verbose", &verbose );

	interface->CheckFlags(argc, argv);

	if( InputFiles.size() == 0 || OutputFile.size() == 0 ) {

		cerr << "You have to provide at least one input file and the output file!" << endl;
		exit(1);

	}

	if( CalibrationFile.size() == 0 ) {

		cout << "No Calibration File given, you have to provide one!" << endl;
		exit(1);

	}

	cout << "input file(s):" << endl;
	for( unsigned int i=0; i<InputFiles.size(); i++ ) {

		cout << InputFiles[i] << endl;
	
	}
	
	cout << "calibration file: " << CalibrationFile << endl;
	cout << "output file: " << OutputFile << endl;

	TChain* tr;
	tr = new TChain("tr");
	for( unsigned int i = 0; i < InputFiles.size(); i++ ) {
	
		tr->Add( InputFiles[i] );
		
	}

	if( tr == NULL ) {

		cout << "could not find tree caltr in file " << endl;

		for( unsigned int i=0; i < InputFiles.size(); i++ ) {

			cout << InputFiles[i] << endl;

		}
		
		return 3;

	}

	Calibration *Cal = new Calibration( CalibrationFile.c_str() );
	if(verbose) Cal->PrintCalibration();
  
	BuiltEvent* event = new BuiltEvent;

	tr->SetBranchAddress( "Event", &event );

	TFile* outfile = new TFile( OutputFile.c_str(), "recreate" );
	if( outfile->IsZombie() ) return 4;
	
	// Write to mb_evts tree
 	mbevts* write_mb_evts = new mbevts();
	mbevts* mb_evts[GAMMA_ARRAY];
	for( int i = 0; i < GAMMA_ARRAY; i++ ) {
	
		mb_evts[i] = new mbevts();
		mb_evts[i]->Initialize();

	}
	
	TTree* g_clx = new TTree( "g_clx", "g_clx" );
	g_clx->Branch( "mbevts", "mbevts", &write_mb_evts );
 
	unsigned int i,j,k,l;
  
	// How many ticks need to align the prompt, in ticks.
	Double_t dtAdc[4] = {14.,16.,17.,9.};  // IS597
  
	// IS597
	Double_t tMinPrompt = -15., tMaxPrompt = 10.;
	Double_t tMinRandom = 15., tMaxRandom = 40.;
 
	Double_t tMinPromptElectron = -1., tMaxPromptElectron = 1.;
	Double_t tMinRandomElectron = 5., tMaxRandomElectron = 7.;
 
	Double_t WeightPR = abs(tMinPrompt-tMaxPrompt)/abs(tMinRandom-tMaxRandom);
	
	if( verbose ) cout << "WeightPR: " << WeightPR << endl;
  
	double tdiffPG = 0.;

	// Which gates do you need in the particle detector.
	// Double_t GEMinAdc[4] = {200.,200.,200.,200.};
	// Double_t GEMaxAdc[4] = {4096.,4096.,4096.,4096.};

	// Variables
	float GammaEnergy = 0.;
	float GammaEnergy2 = 0.;
	float GammaEnergy3 = 0.;
	float GammaEnergy4 = 0.;
	float ElectronEnergy = 0.;
	float PartEnergy = 0.;
	float icE_Energy = 0.;
	float icDE_Energy = 0.;
	float PadEnergy = 0.;
	unsigned int GammaCtr = 0;
	unsigned int PartCtr = 0;
	float MaxSegEnergy = -99.;
	float MaxSegEnergy2 = -99.;
	int MaxSegClu = -1;
	int MaxSegClu2 = -1;
	int MaxSegCore = -1;
	int MaxSegCore2 = -1;
	int MaxSegId = -1;
	int MaxSegId2 = -1;
	int coinc_flag;
	Int_t dgf_num = 0;
	Int_t dgf_num2 = 0;
	Int_t dgf_num3 = 0;
	Int_t dgf_num4 = 0;
	Int_t dgf_ch = 0;
	Int_t dgf_ch2 = 0;
	Int_t dgf_ch3 = 0;
	Int_t dgf_ch4 = 0;
	Int_t dgf_en = 0;
	Int_t dgf_en2 = 0;
	Int_t dgf_en3 = 0;
	Int_t dgf_en4 = 0;
	long long dgf_t = 0;
	long long dgf_t2 = 0;
	long long dgf_t3 = 0;
	long long dgf_t4 = 0;
	Int_t adc_num = 0;
	Int_t adc_ch = 0;
	Int_t adc_en = 0;
	Int_t adc_num2 = 0;
	Int_t adc_ch2 = 0;
	Int_t adc_en2 = 0;
	long long adc_t =0;
	long long adc_t2 =0;
	bool electron_flag = false;
 
	Int_t RingPos1Chan = 18;
	Int_t RingPos2Chan = 19;
	Int_t RingEn1Chan = 16;
	Int_t RingEn2Chan = 17;
	Int_t StripPos1Chan = 22;
	Int_t StripPos2Chan = 23;
	Int_t StripEn1Chan = 20;
	Int_t StripEn2Chan = 21;
	Int_t CDPadChan = 24;
	Int_t BarrelTotEChan = 25;
  
	Double_t Threshold_CDRing_ID[4] = {300.,300.,300.,300.};
	Double_t Threshold_CDRing_E[4] = {55.,55.,55.,55.};
	Double_t Threshold_CDStrip_ID[4] = {300.,300.,300.,300.};
	Double_t Threshold_CDStrip_E[4] = {55.,55.,55.,55.};
	Double_t Threshold_FCDPad[4] = {500.,500.,500.,500.};
	Double_t Threshold_BBarrel[4] = {200.,100.,100.,100.};
	Double_t Threshold_BBarrelFront = 50.;
	Double_t LimtFBCD[4] = {800.,800.,800.,800.};
  
	Int_t DemuxRing = 0;
	Int_t DemuxStrip = 0;
  
	vector<unsigned short> cd_ringenergyhit1[4];
	vector<unsigned short> cd_ringenergyhit2[4];
	vector<unsigned short> cd_stripenergyhit1[4];
	vector<unsigned short> cd_stripenergyhit2[4];
	vector<unsigned short> cd_ringidhit1[4];
	vector<unsigned short> cd_ringidhit2[4];
	vector<unsigned short> cd_stripidhit1[4];
	vector<unsigned short> cd_stripidhit2[4];

	unsigned int cd_ringindex[4];
	unsigned int cd_stripindex[4];
  
	vector<unsigned int> Quad; // 0: Top, 1: Bottom, 2: Left and 3; Right
	vector<unsigned int> Elem_fired; // 0: FCD, 1: Barrel, 2: BCD and 3: Pad
	vector<unsigned int> Chan_front; // Rings for CDs, Total Energy for Pad or Strips for Barrel
	vector<double> Ener_front;
	vector<unsigned int> Chan_back; // Strips for CDs and Total Energy for Barrel
	vector<double> Ener_back;
	vector<long long> time;
	vector<bool> laser;
  
	Int_t LastEntry = 0;
	
	bool HitFCDPad = false;
	bool FCDPadTrig = false;
	bool HitBBarrel = false;
  
	Double_t EnergyBad = 0.;
	Double_t EnergyBarrelBack = 0.;
  
	int* ValuesPosBBarrel;
	int MultBBarrel = 0;
	int StripFront = 0;
	int StripBack = 0;
  
	vector<unsigned short> EnergyBarrelFront(16);
  
	int tempStripNum = 0;
	int tempRingNum = 0;
	int tempStripEnergy = 0;
	int tempRingEnergy = 0;
	
	// Check Code
	Int_t CounterAdcCDFired[5] = {0,0,0,0,0};
  
  
	// ------ Histograms ------ //

	// Histogram errors
	TH1::SetDefaultSumw2();
  
	// diagnostics
	TH1F* adc = new TH1F("adc","adc",8,-0.5,7.5);
	adc->GetXaxis()->SetTitle("Number of ADC's");
	TH1F* dgf = new TH1F("dgf","dgf",56,-0.5,55.5);
	dgf->GetXaxis()->SetTitle("Number of DGF's");
	TH1F* free_dgf = new TH1F("free_dgf","free_dgf",56,-0.5,55.5);
	free_dgf->GetXaxis()->SetTitle("Number of free DGF's");

	// time differences
	TH1F* tdiff_gp = new TH1F("tdiff_gp","tdiff_gp",201,-100.5,100.5);
	tdiff_gp->GetXaxis()->SetTitle("time diff (particle - gamma) [us]");
	TH1F* tdiff_ep = new TH1F("tdiff_ep","tdiff_ep",201,-100.5,100.5);
	tdiff_ep->GetXaxis()->SetTitle("time diff (particle - electron) [us]");
	TH1F* tdiff_gg = new TH1F("tdiff_gg","tdiff_gg",201,-100.5,100.5);
	tdiff_gg->GetXaxis()->SetTitle("time diff between one Dgf and the others [us]");
	TH1F *tdiff_gp_q[4]; 
	for( i = 0; i < 4; i++ ) 
		tdiff_gp_q[i] = new TH1F(Form("tdiff_gp_%d",i),Form("tdiff_gp_%d",i),201,-100.5,100.5);

	// Total gamma - no Doppler correction
	TH1F* E_gam = new TH1F("E_gam","E_gam",GBINS,GMIN,GMAX);
	E_gam->GetXaxis()->SetTitle("Energy Gamma Rays [Channels]");

	// Gamma spectra for every segment - no Doppler correction
	TH1F* E_gam_seg[8][3][7];
	TH1F* E_gam_seg_cal[8][3][7];
	TDirectory *gam_dir = outfile->mkdir("E_gam_seg");
	gam_dir->cd();
	for( i = 0; i < 8; i++ ) {

		for( j = 0; j < 3; j++ ) {

			for( k = 0; k < 7; k++ ) {

				E_gam_seg[i][j][k] = new TH1F(Form("E_gam_%d_%d_%d",i,j,k),Form("E_gam_%d_%d_%d",i,j,k),16384,-0.5,65535.5);
				E_gam_seg[i][j][k]->GetXaxis()->SetTitle("Energy Gamma Rays [Channels]");
				E_gam_seg_cal[i][j][k] = new TH1F(Form("E_gam_%d_%d_%d_cal",i,j,k),Form("E_gam_%d_%d_%d_cal",i,j,k),GBINS,GMIN,GMAX);
				E_gam_seg_cal[i][j][k]->GetXaxis()->SetTitle("Energy Gamma Rays [keV]");

			}

		}

	}
	
	gDirectory->cd("/");
	
	// Particle spectra for every segment
	TH1F* E_part_ann[4][16];
	TH1F* E_part_ann_cal[4][12];
	TH1F* E_part_sec[4][12];
	TH1F* E_part_sec_cal[4][12];
	TDirectory *part_dir = outfile->mkdir("E_part");
	part_dir->cd();
	for( i = 0; i < 4; i++ ) {

		for( j = 0; j < 16; j++ ) {

			E_part_ann[i][j] = new TH1F(Form("E_part_ann_%d_%d",i,j),Form("E_part_ann_%d_%d",i,j),4096,-0.5,4095.5);
			E_part_ann[i][j]->GetXaxis()->SetTitle("Energy Particles [Channels]");
			E_part_ann_cal[i][j] = new TH1F(Form("E_part_ann_%d_%d_cal",i,j),Form("E_part_ann_%d_%d_cal",i,j),PBINS,PMIN,PMAX);
			E_part_ann_cal[i][j]->GetXaxis()->SetTitle("Energy Particles [keV]");

		}

		for( j = 0; j < 12; j++ ) {

			E_part_sec[i][j] = new TH1F(Form("E_part_sec_%d_%d",i,j),Form("E_part_sec_%d_%d",i,j),4096,-0.5,4095.);
			E_part_sec[i][j]->GetXaxis()->SetTitle("Energy Particles [Channels]");
			E_part_sec_cal[i][j] = new TH1F(Form("E_part_sec_%d_%d_cal",i,j),Form("E_part_sec_%d_%d_cal",i,j),PBINS,PMIN,PMAX);
			E_part_sec_cal[i][j]->GetXaxis()->SetTitle("Energy Particles [keV]");

		}

	}
	
	gDirectory->cd("/");
	
	TH1F* E_spede;
	TH1F* E_spede_seg[24];
	TH1F* E_spede_seg_cal[24];
	if( Spede ) {
		
		// Total electron - no Doppler correction
		E_spede = new TH1F("E_spede","E_spede",ELBINS,ELMIN,ELMAX);
		E_spede->GetXaxis()->SetTitle("Energy Electrons [Channels]");
		
		// Electron spectra for every segment - no Doppler correction
		TDirectory *spede_dir = outfile->mkdir("E_spede_seg");
		spede_dir->cd();
		for( i = 0; i < 24; i++ ) {
			
			E_spede_seg[i] = new TH1F(Form("E_spede_%d",i),Form("E_spede_%d",i),16384,-0.5,65535.5);
			E_spede_seg[i]->GetXaxis()->SetTitle("Energy Electrons [Channels]");
			E_spede_seg_cal[i] = new TH1F(Form("E_spede_%d_cal",i),Form("E_spede_%d_cal",i),ELBINS,ELMIN,ELMAX);
			E_spede_seg_cal[i]->GetXaxis()->SetTitle("Energy Electrons [keV]");
			
		}
		
		gDirectory->cd("/");
		
	}

	TH1F* E_BeamDump = new TH1F("E_BeamDump","E_BeamDump",GBINS,GMIN,GMAX);
	E_BeamDump->GetXaxis()->SetTitle("Energy of the Beam Dump [keV]");
	TH1F* T_BeamDump = new TH1F("T_BeamDump","T_BeamDump",7200,0,7200);	
	T_BeamDump->GetXaxis()->SetTitle("Time of the Beam Dump [s]");

	// particle-spectra
	TH2F* FBCD_Rings_id_energy_hit1[4];
	TH2F* FBCD_Rings_id_energy_hit2[4];
	TH2F* FBCD_Strips_id_energy_hit1[4];
	TH2F* FBCD_Strips_id_energy_hit2[4];
	TH2F *CD_front_energy[4], *CD_front_energy_cal[4];
	TH2F *CD_back_energy[4], *CD_back_energy_cal[4];

	if( crex ) {
		
		for( i = 0; i < 4; i++ ) {
			
			FBCD_Rings_id_energy_hit1[i] = new TH2F(Form("FBCD_Rings_id_energy_hit1_%d",i),Form("FBCD_Rings_id_energy_hit1_%d",i),1200,300,1500,4096,0,4096);
			FBCD_Rings_id_energy_hit1[i]->GetXaxis()->SetTitle("Energy ID MUX");
			FBCD_Rings_id_energy_hit1[i]->GetYaxis()->SetTitle("Energy particle [Channels]");
			FBCD_Rings_id_energy_hit2[i] = new TH2F(Form("FBCD_Rings_id_energy_hit2_%d",i),Form("FBCD_Rings_id_energy_hit2_%d",i),1200,300,1500,4096,0,4096);
			FBCD_Rings_id_energy_hit2[i]->GetXaxis()->SetTitle("Energy ID MUX");
			FBCD_Rings_id_energy_hit2[i]->GetYaxis()->SetTitle("Energy particle [Channels]");
			FBCD_Strips_id_energy_hit1[i] = new TH2F(Form("FBCD_Strips_id_energy_hit1_%d",i),Form("FBCD_Strips_id_energy_hit1_%d",i),1200,300,1500,4096,0,4096);
			FBCD_Strips_id_energy_hit1[i]->GetXaxis()->SetTitle("Energy ID MUX");
			FBCD_Strips_id_energy_hit1[i]->GetYaxis()->SetTitle("Energy particle [Channels]");
			FBCD_Strips_id_energy_hit2[i] = new TH2F(Form("FBCD_Strips_energy_id_hit2_%d",i),Form("FBCD_Strips_id_energy_hit2_%d",i),1200,300,1500,4096,0,4096);
			FBCD_Strips_id_energy_hit2[i]->GetXaxis()->SetTitle("Energy ID MUX");
			FBCD_Strips_id_energy_hit2[i]->GetYaxis()->SetTitle("Energy particle [Channels]");
			
		}
		
	}
	
	else {
	
		for( i = 0; i < 4; i++ ) {
			
			CD_front_energy[i] = new TH2F(Form("CD_front_energy_%d",i),Form("CD_front_energy_%d",i),16,-0.5,15.5,4096,0,4096);
			CD_front_energy[i]->GetXaxis()->SetTitle("Ring number");
			CD_front_energy[i]->GetYaxis()->SetTitle("Energy particle [adc ch.]");
			CD_back_energy[i] = new TH2F(Form("CD_back_energy_%d",i),Form("CD_back_energy_%d",i),16,-0.5,15.5,4096,0,4096);
			CD_back_energy[i]->GetXaxis()->SetTitle("Strip number");
			CD_back_energy[i]->GetYaxis()->SetTitle("Energy particle [adc ch.]");
			
			CD_front_energy_cal[i] = new TH2F(Form("CD_front_energy_cal_%d",i),Form("CD_front_energy_cal_%d",i),16,-0.5,15.5,PBINS,PMIN,PMAX);
			CD_front_energy_cal[i]->GetXaxis()->SetTitle("Ring number");
			CD_front_energy_cal[i]->GetYaxis()->SetTitle("Energy particle [MeV]");
			CD_back_energy_cal[i] = new TH2F(Form("CD_back_energy_cal_%d",i),Form("CD_back_energy_cal_%d",i),16,-0.5,15.5,PBINS,PMIN,PMAX);
			CD_back_energy_cal[i]->GetXaxis()->SetTitle("Strip number");
			CD_back_energy_cal[i]->GetYaxis()->SetTitle("Energy particle [MeV]");
			
		}
		
	}

	// Ionisation chamber
	TH2F* dEE;
	if( ionch ) dEE = new TH2F( "dEE", "ionisation chamber;E_{rest};delta-E", 4096, -0.5, 4095.5, 4096, -0.5, 4095.5 );

	// CD-pad detector, freely triggered
	TH1F *padE_sum, *padE[4], *padE_cal[4];
	TH2F *cd_dEE_sum, *cd_dEE[4];
	if( cdpad ) {
	
		padE_sum = new TH1F( "padE_sum", "Energy on the PAD detector;Energy [keV];", 4096, -0.5, 4095.5 );
		cd_dEE_sum = new TH2F( "cd_dEE_sum", "dE-E of the CD-PAD;Energy [keV];Energy [keV];", 4096, -2., 16382., 4096, -2., 16382. );

		for( i = 0; i < 4; i++ ) {

			padE[i] = new TH1F( Form("padE_%d",i), "Energy on the PAD detector;Energy [adc ch.];", 4096, -0.5, 4095.5 );
			padE_cal[i] = new TH1F( Form("padE%d_cal",i), "Energy on the PAD detector;Energy [keV];", ELBINS, ELMIN, ELMAX );
			cd_dEE[i] = new TH2F( Form("cd_dEE%d",i), "dE-E of the CD-PAD;Energy [keV];Energy [keV];", 4096, -2., 16382., 4096, -2., 16382. );

		}
		
	}

	Double_t nentries = tr->GetEntries();
	Int_t nbytes = 0;
	Int_t status;
	vector<int> DgfModule;
	
	// Test - comment out for runs
	nentries = 100000;
	
	// Start loop over number of entries
	for( i = 0; i < nentries; i++ ) {

		status = tr->GetEvent(i);

		if( status == -1 ) {

			cerr << "Error occured, couldn't read entry " << i << " from tree ";
			cerr << tr->GetName() << " in file " << tr->GetFile()->GetName() << endl;
			return 5;
			
		}
		
		else if( status == 0 ) {

			cerr << "Error occured, entry " << i << " in tree " << tr->GetName();
			cerr << " in file " << tr->GetFile()->GetName() << " doesn't exist" << endl;
			return 6;

		}
		
		nbytes += status;

		// Sort out mb_evts
		for( j = 0; j < GammaCtr; j++ ) {
		
			mb_evts[j]->SearchCoin();  
			write_mb_evts->Initialize();
			write_mb_evts->CopyData( mb_evts[j] );
			if( write_mb_evts->pr_hits > 0 || write_mb_evts->rndm_hits > 0 ) g_clx->Fill(); 
			else if( Singles ) g_clx->Fill();
			
		}

		// Reset
		for( j = 0; j < GAMMA_ARRAY; j++ ) {
		
			mb_evts[j]->Initialize();
		
		}
		GammaCtr = 0;


		DgfModule.clear();

		for( j = 0; j < event->NumberOfDgfs(); j++ ) {

			for( k = 0; k < DgfModule.size(); k++ ) 

				if( event->Dgf(j)->ModuleNumber() == DgfModule[k] ) break;
			
			if( k == DgfModule.size() ) DgfModule.push_back( event->Dgf(j)->ModuleNumber() );

		}
	
		for( j = 0; j < 4; j++ ) {

			cd_ringenergyhit1[j].clear();
			cd_ringenergyhit2[j].clear();
			cd_stripenergyhit1[j].clear();
			cd_stripenergyhit2[j].clear();
			cd_ringidhit1[j].clear();
			cd_ringidhit2[j].clear();
			cd_stripidhit1[j].clear();
			cd_stripidhit2[j].clear();

		}
	
		Quad.clear();
		Elem_fired.clear();
		Chan_front.clear();
		Ener_front.clear();
		Chan_back.clear();
		Ener_back.clear();
		time.clear();
		laser.clear();
	  
		adc->Fill( event->NumberOfAdcs() );
		dgf->Fill( DgfModule.size() );
		  
		if( event->NumberOfAdcs() == 0 ) {
	
			free_dgf->Fill( DgfModule.size() );

			for( j = 0; j < event->NumberOfDgfs(); j++ ) {

				for( k = j+1; k < event->NumberOfDgfs(); k++ ) {

					tdiff_gg->Fill( event->Dgf(j)->Time() - event->Dgf(k)->Time() );

				}

			}

		}

		// Only Gammas (dgfs)
		for( j = 0; j < event->NumberOfDgfs(); j++ ) {

			dgf_num = event->Dgf(j)->ModuleNumber();
			dgf_ch  = event->Dgf(j)->Channel();
			dgf_en  = event->Dgf(j)->Energy();
			dgf_t   = event->Dgf(j)->Time();
				
			if( 0 <= dgf_num && dgf_num < 48 && 0 <= dgf_ch && dgf_ch < 4 ) { // miniball

				GammaEnergy = Cal->DgfEnergy( dgf_num, dgf_ch, dgf_en );

				if( dgf_num % 2 == 0 && dgf_ch < 3 ) { // cores plus seg1 and seg2

					E_gam_seg[dgf_num/6][dgf_num%6/2][dgf_ch]->Fill( dgf_en );
					E_gam_seg_cal[dgf_num/6][dgf_num%6/2][dgf_ch]->Fill( GammaEnergy );
					if( dgf_ch == 0 ) E_gam->Fill( GammaEnergy );

				} // even DGF number

				else {

					E_gam_seg[dgf_num/6][dgf_num%6/2][dgf_ch+3]->Fill( dgf_en );
					E_gam_seg_cal[dgf_num/6][dgf_num%6/2][dgf_ch+3]->Fill( GammaEnergy );

				} // odd DGF number
					
			}

			else if( dgf_num == 53 && dgf_ch == 0 ) {

				GammaEnergy = Cal->DgfEnergy( dgf_num, dgf_ch, dgf_en );
				E_BeamDump->Fill(Cal->DgfEnergy(53,0,dgf_en));
				T_BeamDump->Fill((dgf_t)/40000000);

			}

		}
		
		if( event->NumberOfAdcs() > 0 )
			cout << "No ADCs = " << event->NumberOfAdcs() << endl;

		// Only Particles (and SPEDE if it is in ADC5)
		for( j = 0; j < event->NumberOfAdcs(); j++ ) {

			HitFCDPad = false;
			HitBBarrel = false;
			EnergyBarrelFront.assign( EnergyBarrelFront.size(), 0 );

			adc_num = event->Adc(j)->ModuleNumber();
			cd_ringindex[adc_num] = 0;
			cd_stripindex[adc_num] = 0;

			for( k = 0; k < event->Adc(j)->SubEvent()->Size(); k++ ) {

				adc_ch = event->Adc(j)->SubEvent()->AdcChannel(k);
				adc_en = event->Adc(j)->SubEvent()->AdcValue(k);

				if( crex && adc_num >= 0 && adc_num < 4 && adc_ch >= 0 && adc_ch < 26 ) { // CREX

					if( adc_ch < 16 ) {

						if( adc_en > Threshold_BBarrelFront && adc_en < 3840 )

							EnergyBarrelFront[adc_ch] = adc_en;
					
					}

					else if( adc_ch == CDPadChan ) {

						if( adc_en > Threshold_FCDPad[adc_num] ) {

							HitFCDPad = true;
							EnergyBad = adc_en;

						}

					}
					
					else if( adc_ch == BarrelTotEChan ) {

						if( adc_en > Threshold_BBarrel[adc_num] && adc_en < 3840 ) {

							HitBBarrel = true;
							EnergyBarrelBack = adc_en;

						}

					}

					else if( adc_ch == RingEn1Chan )
						cd_ringenergyhit1[adc_num].push_back(adc_en);

					else if( adc_ch == RingEn2Chan)
						cd_ringenergyhit2[adc_num].push_back(adc_en);

					else if( adc_ch == RingPos1Chan)
						cd_ringidhit1[adc_num].push_back(adc_en);

					else if( adc_ch == RingPos2Chan)
						cd_ringidhit2[adc_num].push_back(adc_en);

					else if( adc_ch == StripEn1Chan)
						cd_stripenergyhit1[adc_num].push_back(adc_en);

					else if( adc_ch == StripEn2Chan)
						cd_stripenergyhit2[adc_num].push_back(adc_en);

					else if(adc_ch == StripPos1Chan)
						cd_stripidhit1[adc_num].push_back(adc_en);

					else if(adc_ch == StripPos2Chan)
						cd_stripidhit2[adc_num].push_back(adc_en);

				} // particle in CREX
				
				else if( !crex && adc_num >= 0 && adc_num < 4 ) { // CD
					
					if( adc_ch < 16 ) {
						
						cd_ringenergyhit1[adc_num].push_back( adc_en );
						cd_ringidhit1[adc_num].push_back( adc_ch );
						PartEnergy = Cal->AdcEnergy( adc_num, adc_ch, adc_en );
						E_part_ann[adc_num][adc_ch]->Fill( adc_en );
						E_part_ann_cal[adc_num][adc_ch]->Fill( PartEnergy );
						
						if( tempRingEnergy < adc_en ) {
							
							tempRingEnergy = adc_en;
							tempRingNum = adc_ch;
							cd_ringindex[adc_num] = cd_ringenergyhit1[adc_num].size()-1;
							
						}
						
					}
					
					else if( adc_ch < 28 ) {
						
						cd_stripenergyhit1[adc_num].push_back( adc_en );
						cd_stripidhit1[adc_num].push_back( adc_ch );
						PartEnergy = Cal->AdcEnergy( adc_num, adc_ch, adc_en );
						E_part_sec[adc_num][adc_ch-16]->Fill( adc_en );
						E_part_sec_cal[adc_num][adc_ch-16]->Fill( PartEnergy );
						
						if( tempStripEnergy < adc_en ) {
							
							tempStripEnergy = adc_en;
							tempStripNum = adc_ch-16;
							cd_stripindex[adc_num] = cd_stripenergyhit1[adc_num].size()-1;
							
						}

						if( cdpad ){

							for( l = 0; l < event->Adc(j)->SubEvent()->Size(); l++ ){

								adc_ch2 = event->Adc(j)->SubEvent()->AdcChannel(l);
								adc_en2 = event->Adc(j)->SubEvent()->AdcValue(l);
								adc_t2 = event->Adc(j)->Time();

								if( adc_ch2 != 31 ) continue;

								PadEnergy = Cal->AdcEnergy( adc_num, adc_ch2, adc_en2 );

								cd_dEE[adc_num]->Fill( PadEnergy, PartEnergy );
								cd_dEE_sum->Fill( PadEnergy, PartEnergy );

							}	

						}

					}

					else if( adc_ch == 31 && cdpad ) { // something in the pad!

						PadEnergy = Cal->AdcEnergy( adc_num, adc_ch, adc_en );
					
						padE[adc_num]->Fill( adc_en );
						padE_cal[adc_num]->Fill( PadEnergy );
						padE_sum->Fill( PadEnergy );
						
					}
					
				} // particle in CD
			
				else if( adc_num == 4 && Spede ) { // SPEDE
					
					ElectronEnergy = Cal->AdcEnergy( 4, adc_ch, adc_en );
					
					// STM-16 one
					if( adc_ch < 12 ) {
						
						E_spede_seg[adc_ch]->Fill( adc_en );
						E_spede_seg_cal[adc_ch]->Fill( ElectronEnergy );
						E_spede->Fill( ElectronEnergy );
					
					}
					
					// STM-16 two
					else if( adc_ch > 15 && adc_ch < 28 ) {
						
						E_spede_seg[adc_ch-4]->Fill( adc_en );
						E_spede_seg_cal[adc_ch-4]->Fill( ElectronEnergy );
						E_spede->Fill( ElectronEnergy );
						
					}
					
				} // SPEDE
				
				else if( adc_num == 4 && ionch && adc_ch == 0 ) { // ionisation chamber (gas trigger)
				
					icDE_Energy = Cal->AdcEnergy( 4, adc_ch, adc_en );
					
					for( l = 0; l < event->Adc(j)->SubEvent()->Size(); l++ ){

						adc_ch2 = event->Adc(j)->SubEvent()->AdcChannel(l);
						adc_en2 = event->Adc(j)->SubEvent()->AdcValue(l);

						if( adc_ch2 != 1 ) continue;

						icE_Energy = Cal->AdcEnergy( 4, adc_ch2, adc_en2 );

						dEE->Fill( icE_Energy, icDE_Energy );

					}

				} // ionisation chamber
				
			} // k
			
			// if we did have SPEDE (or ion. chamber or pad), don't carry on with particle stuff
			//if( adc_num == 4 || ( adc_ch == 31 && cdpad ) ) continue;
			if( adc_num == 4 ) continue;
			
			if( crex ) {
				
				LastEntry = cd_ringenergyhit1[adc_num].size() - 1;
				
				if( HitBBarrel ) {
					
					ValuesPosBBarrel = PosBBarrel(adc_num, EnergyBarrelFront, EnergyBarrelBack);
					MultBBarrel = ValuesPosBBarrel[0];
					StripFront = ValuesPosBBarrel[1];
					StripBack = ValuesPosBBarrel[2];
					
					if( MultBBarrel == 0 ) HitBBarrel = false;
					
				} // HitBBarrel
		  
				// Veto if the Pad is not in the trigger
				if( !NoPad && HitFCDPad ) FCDPadTrig = true;
				else FCDPadTrig = false;
				
				// Only hit in one of the CDs And veto if the Pad is not in the trigger
				if( !FCDPadTrig && ( !HitBBarrel || OnlyFBCD ) &&
				   cd_ringenergyhit1[adc_num].size() > 0 && adc_num < 4) {
					
					LastEntry = cd_ringenergyhit1[adc_num].size() - 1;
					CounterAdcCDFired[adc_num]++;
					
					DemuxRing = Cal->PosFBCDRing(adc_num,cd_ringidhit1[adc_num][LastEntry]);
					DemuxStrip = Cal->PosFBCDStrip(adc_num,cd_stripidhit1[adc_num][LastEntry]);
					
					if( cd_ringidhit1[adc_num][LastEntry] > Threshold_CDRing_ID[adc_num] &&
					   cd_ringenergyhit1[adc_num][LastEntry] > Threshold_CDRing_E[adc_num] &&
					   cd_stripidhit1[adc_num][LastEntry] > Threshold_CDStrip_ID[adc_num] &&
					   cd_stripenergyhit1[adc_num][LastEntry] > Threshold_CDStrip_E[adc_num] &&
					   cd_ringidhit1[adc_num][LastEntry] < LimtFBCD[adc_num] &&
					   cd_stripidhit1[adc_num][LastEntry] < LimtFBCD[adc_num] &&
					   cd_ringidhit1[adc_num][LastEntry] < 3840 &&
					   cd_stripidhit1[adc_num][LastEntry] < 3840 &&
					   cd_ringenergyhit1[adc_num][LastEntry] < 3840 &&
					   cd_stripenergyhit1[adc_num][LastEntry] < 3840 &&
					   DemuxRing < 16 && DemuxStrip < 16 ) {
						
						// The FCD was fired
						if( cd_ringidhit2[adc_num][LastEntry] < Threshold_CDRing_ID[adc_num]
						   && cd_stripidhit2[adc_num][LastEntry] < Threshold_CDStrip_ID[adc_num] ) {
							
							// 1 ring and 1 strip fired, no reconstruction energy
							Quad.push_back(adc_num);
							Elem_fired.push_back(0);
							Chan_front.push_back(DemuxRing);
							Ener_front.push_back(Cal->AdcEnergy(adc_num, Chan_front.back(), cd_ringenergyhit1[adc_num][LastEntry]));
							Chan_back.push_back(DemuxStrip);
							Ener_back.push_back(cd_stripenergyhit1[adc_num][LastEntry]);
							
							time.push_back(event->Adc(j)->Time() + dtAdc[adc_num]);
							laser.push_back(event->Adc(j)->LaserOn());
							
						}
						
					}
					
					else if( cd_ringidhit1[adc_num][LastEntry] > Threshold_CDRing_ID[adc_num]
							&& cd_ringenergyhit1[adc_num][LastEntry] > Threshold_CDRing_E[adc_num]
							&& cd_stripidhit1[adc_num][LastEntry] > Threshold_CDStrip_ID[adc_num]
							&& cd_stripenergyhit1[adc_num][LastEntry] > Threshold_CDStrip_E[adc_num]
							&& ( cd_ringidhit1[adc_num][LastEntry] > LimtFBCD[adc_num] && cd_stripidhit1[adc_num][LastEntry] > LimtFBCD[adc_num] )
							&& ( cd_ringidhit1[adc_num][LastEntry] < 3840 && cd_stripidhit1[adc_num][LastEntry] < 3840 )
							&& ( cd_ringenergyhit1[adc_num][LastEntry] < 3840 && cd_stripenergyhit1[adc_num][LastEntry] < 3840 )
							&& DemuxRing < 16 && DemuxStrip < 16 && !VetoBCD ) {
						
						// The BCD was fired
						if( cd_ringidhit2[adc_num][LastEntry] < Threshold_CDRing_ID[adc_num]
						   && cd_stripidhit2[adc_num][LastEntry] < Threshold_CDRing_ID[adc_num] ) {
							
							// 1 ring and 1 strip fired, no reconstruction energy
							Quad.push_back(adc_num);
							Elem_fired.push_back(2);
							Chan_front.push_back(DemuxRing);
							Ener_front.push_back(cd_ringenergyhit1[adc_num][LastEntry]);
							Chan_back.push_back(DemuxStrip);
							Ener_back.push_back(cd_stripenergyhit1[adc_num][LastEntry]);
							
							time.push_back(event->Adc(j)->Time() + dtAdc[adc_num]);
							laser.push_back(event->Adc(j)->LaserOn());
							
						}
						
					}
					
				}
				
				// Only hit in the BBarrel And veto if the Pad is not in the trigger
				else if( !FCDPadTrig && HitBBarrel && adc_num < 4 ) {
					
					if( MultBBarrel == 1 ) {
						
						Quad.push_back(adc_num);
						Elem_fired.push_back(1);
						Chan_front.push_back(StripFront);
						Ener_front.push_back(EnergyBarrelBack);
						Chan_back.push_back(StripBack); // ratio: EneFirstStripHit/EnergyBarrelBack
						Ener_back.push_back(EnergyBarrelBack);
						
						time.push_back(event->Adc(j)->Time() + dtAdc[adc_num]);
						laser.push_back(event->Adc(j)->LaserOn());
						
					}
					
				}
				
				// Only hit in the FCD Pad and the Pad is in the trigger
				else if( FCDPadTrig && HitFCDPad && !HitBBarrel && adc_num < 4 ) {
					
					Quad.push_back(adc_num);
					Elem_fired.push_back(3);
					Chan_front.push_back(0);
					Ener_front.push_back(EnergyBad);
					Chan_back.push_back(0);
					Ener_back.push_back(0);
					
					time.push_back(event->Adc(j)->Time() + dtAdc[adc_num]);
					laser.push_back(event->Adc(j)->LaserOn());
					
				}
				
			} // crex

			else { // CD
			
				// Use only the highest energy hit - no reconstruction
				if( tempRingEnergy > Threshold_CDRing_E[adc_num] &&
				 	tempStripEnergy > Threshold_CDStrip_E[adc_num] &&
				  	adc_num >= 0 && adc_num < 4 && 
					tempRingNum >= 0 && tempRingNum < 16 &&
					tempStripNum >= 0 && tempStripNum < 12 ) {

					Quad.push_back( adc_num );
					Elem_fired.push_back(0);
					Chan_front.push_back( tempRingNum );
					Chan_back.push_back( tempStripNum );
					Ener_front.push_back( Cal->AdcEnergy( adc_num, Chan_front.back(), tempRingEnergy ) );
					Ener_back.push_back( Cal->AdcEnergy( adc_num, Chan_back.back(), tempStripEnergy ) );
					
					time.push_back(event->Adc(j)->Time() + dtAdc[adc_num]);
					laser.push_back(event->Adc(j)->LaserOn());
					
					CounterAdcCDFired[adc_num]++;
					
				}
				
			} // CD
			
		} // j - numberofadcs
	
		// Loop over quadrants
		for( j = 0; j < 4; j++ ) {
	
			// Mux if it's CREX
			if( crex ) {
				
				for( k = 0; k < cd_ringenergyhit1[j].size(); k++ )
					FBCD_Rings_id_energy_hit1[j]->Fill(cd_ringidhit1[j][k], cd_ringenergyhit1[j][k]);
				
				for( k = 0; k < cd_ringenergyhit2[j].size(); k++ )
					FBCD_Rings_id_energy_hit2[j]->Fill(cd_ringidhit2[j][k], cd_ringenergyhit2[j][k]);
				
				for( k = 0; k < cd_stripenergyhit1[j].size(); k++ )
					FBCD_Strips_id_energy_hit1[j]->Fill(cd_stripidhit1[j][k], cd_stripenergyhit1[j][k]);
				
				for( k = 0; k < cd_stripenergyhit2[j].size(); k++ )
					FBCD_Strips_id_energy_hit2[j]->Fill(cd_stripidhit2[j][k], cd_stripenergyhit2[j][k]);
				
			} // CREX
			
			else { // CD
				
				for( k = 0; k < cd_ringenergyhit1[j].size(); k++ ) {

					PartEnergy = Cal->AdcEnergy( j, cd_ringidhit1[j][k], cd_ringenergyhit1[j][k] );
					CD_front_energy[j]->Fill( cd_ringidhit1[j][k], cd_ringenergyhit1[j][k] );
					CD_front_energy_cal[j]->Fill( cd_ringidhit1[j][k], PartEnergy/1000. );
				
				}

				for( k = 0; k < cd_stripenergyhit1[j].size(); k++ ) {

					PartEnergy = Cal->AdcEnergy( j, cd_stripidhit1[j][k], cd_stripenergyhit1[j][k] );
					CD_back_energy[j]->Fill( cd_stripidhit1[j][k]-16, cd_stripenergyhit1[j][k] );
					CD_back_energy[j]->Fill( cd_stripidhit1[j][k]-16, PartEnergy/1000. );

				}

			} // CD
			
		} // j

		// Gamma-particle coincidences
		for( j = 0; j < event->NumberOfDgfs(); j++ ) {
	
			dgf_num = event->Dgf(j)->ModuleNumber();
			dgf_ch  = event->Dgf(j)->Channel();
			dgf_en  = event->Dgf(j)->Energy();
			dgf_t   = event->Dgf(j)->Time();		
		  
		  	// Look for a core event
			if( 0 <= dgf_num && dgf_num < 48 && dgf_ch == 0 && dgf_num % 2 == 0 ) {
			
				GammaEnergy = Cal->DgfEnergy( dgf_num, dgf_ch, dgf_en );
				MaxSegClu = dgf_num / 6;
				MaxSegCore = dgf_num % 6 / 2;						

				// Check for highest energy segment in same detector
				for( k = 0; k < event->NumberOfDgfs(); k++ ) {
			
					dgf_num2 = event->Dgf(k)->ModuleNumber();
					dgf_ch2  = event->Dgf(k)->Channel();
					dgf_en2  = event->Dgf(k)->Energy();
					dgf_t2   = event->Dgf(k)->Time();
					
					// We don't care if it's the same event
					if( k == j ) continue;
			
					// Skip if a different detector
					if( dgf_num2 != dgf_num && dgf_num2 != dgf_num + 1 ) continue;
					
					GammaEnergy2 = Cal->DgfEnergy( dgf_num2, dgf_ch2, dgf_en2 );

					// Test maximum energy segment
					if( GammaEnergy2 < MaxSegEnergy ) continue;
					
					// Reassign energy and id
					MaxSegEnergy = GammaEnergy2;
					if( dgf_num2 % 2 == 0 ) MaxSegId = dgf_ch2 - 1;
					else MaxSegId = dgf_ch2+3 - 1;					
			
				} // k

				// Found highest energy segment
				mb_evts[GammaCtr]->SetGen( GammaEnergy );
				mb_evts[GammaCtr]->SetCluid( MaxSegClu );
				mb_evts[GammaCtr]->SetCid( MaxSegClu*3 + MaxSegCore );
				mb_evts[GammaCtr]->SetSid( MaxSegId );
				mb_evts[GammaCtr]->SetTheta( 0. );
				mb_evts[GammaCtr]->SetPhi( 0. );
					
				// Do particles
				for( k = 0; k < Elem_fired.size(); k++ ) {

					tdiffPG = time[k] - dgf_t;

					tdiff_gp->Fill( tdiffPG );
					tdiff_gp_q[(int)Quad[k]]->Fill( tdiffPG - dtAdc[(int)Quad[k]] );

					if( tMinRandom <= tdiffPG && tdiffPG <= tMaxRandom ) coinc_flag = 1;
					else if( tMinPrompt <= tdiffPG && tdiffPG <= tMaxPrompt ) coinc_flag = 0;
					else coinc_flag = -1;			
					
					
					// Deal with the front CD only with mb_evts (for now)
					if( Elem_fired[k] != 0 ) continue;

					// Add particle
					mb_evts[GammaCtr]->SetPart( Ener_front[k], (int)Chan_front[k], (int)Chan_back[k], (double)time[k],
						(double)event->SuperCycleTime()/1000000, (double)tdiffPG, (int)coinc_flag, Quad[k], (int)laser[k], PartCtr );
						
					PartCtr++;

				} // End loop All the particles

				// Look for correlated gammas
				for( l = 0; l < event->NumberOfDgfs(); l++ ) {
					
					// skip if it's the same event as before
					if( l == j ) continue;
					
					dgf_num3 = event->Dgf(l)->ModuleNumber();
					dgf_ch3  = event->Dgf(l)->Channel();
					dgf_en3  = event->Dgf(l)->Energy();
					dgf_t3   = event->Dgf(l)->Time();
					
					// skip if it's the same core as before
					if( dgf_num3 == dgf_num ) continue;
					
					// Look for a core event
					if( 6 <= dgf_num3 && dgf_num3 < 48 && dgf_ch3 == 0 && dgf_num3 % 2 == 0 ) { // miniball
						
						GammaEnergy3 = Cal->DgfEnergy( dgf_num3, dgf_ch3, dgf_en3 );
						MaxSegClu2 = dgf_num3 / 6;
						MaxSegCore2 = dgf_num3 % 6 / 2;
						
						// Check for highest energy segment in same detector
						for( k = 0; k < event->NumberOfDgfs(); k++ ) {
							
							dgf_num4 = event->Dgf(k)->ModuleNumber();
							dgf_ch4  = event->Dgf(k)->Channel();
							dgf_en4  = event->Dgf(k)->Energy();
							dgf_t4   = event->Dgf(k)->Time();
							
							// We don't care if it's the same event
							if( k == l ) continue;
							
							// Skip if a different detector
							if( dgf_num4 != dgf_num3 && dgf_num4 != dgf_num3 + 1 ) continue;
							
							GammaEnergy4 = Cal->DgfEnergy( dgf_num4, dgf_ch4, dgf_en4 );
							
							// Test maximum energy segment
							if( GammaEnergy4 < MaxSegEnergy2 ) continue;
							
							// Reassign energy and id
							MaxSegEnergy2 = GammaEnergy4;
							if( dgf_num4 % 2 == 0 ) MaxSegId2 = dgf_ch4 - 1;
							else MaxSegId2 = dgf_ch4+3 - 1;
							
						} // k
						
						// Found highest energy segment
						mb_evts[GammaCtr]->SetCorGamGen( GammaEnergy3 );
						mb_evts[GammaCtr]->SetCorGamCluid( MaxSegClu2 );
						mb_evts[GammaCtr]->SetCorGamCid( MaxSegClu2*3 + MaxSegCore2 );
						mb_evts[GammaCtr]->SetCorGamSid( MaxSegId2 );
						mb_evts[GammaCtr]->SetCorGamGtd( dgf_t3 - dgf_t );
						mb_evts[GammaCtr]->SetCorGamTheta( 0. );
						mb_evts[GammaCtr]->SetCorGamPhi( 0. );
						
					} // real event in the core? 2nd gamma
					
					// Reset
					MaxSegEnergy2 = -99.;
					MaxSegClu2 = -1;
					MaxSegCore2 = -1;
					MaxSegId2 = -1;
					
				} // End search for correlated gammas
				
				// Search for correlated electrons
				for( l = 0; l < event->NumberOfAdcs(); l++ ) {
					
					for( k = 0; k < event->Adc(l)->SubEvent()->Size(); k++ ) {

						adc_num = event->Adc(l)->ModuleNumber();
						
						if( adc_num == 4 && Spede ) { // spede
							
							adc_ch  = event->Adc(l)->SubEvent()->AdcChannel(k);
							adc_en  = event->Adc(l)->SubEvent()->AdcValue(k) ;
							adc_t   = event->Adc(l)->Time();
							
							ElectronEnergy = Cal->AdcEnergy( 4, adc_ch, adc_en );
							
							if( ElectronEnergy < 5.0 || adc_en <= 1 ) continue;
							
							mb_evts[GammaCtr]->SetCorGamGen( ElectronEnergy );
							mb_evts[GammaCtr]->SetCorGamCluid( 8 );
							mb_evts[GammaCtr]->SetCorGamCid( 0 );
							mb_evts[GammaCtr]->SetCorGamSid( adc_ch );
							mb_evts[GammaCtr]->SetCorGamGtd( adc_t - dgf_t );
							mb_evts[GammaCtr]->SetCorGamTheta( 0. );
							mb_evts[GammaCtr]->SetCorGamPhi( 0. );
							
						} // spede

						else if( adc_num >= 0 && adc_num < 4 && adc_ch == 31 && cdpad ){

							adc_ch  = event->Adc(l)->SubEvent()->AdcChannel(k);
							adc_en  = event->Adc(l)->SubEvent()->AdcValue(k) ;
							adc_t   = event->Adc(l)->Time();
							
							ElectronEnergy = Cal->AdcEnergy( 4, adc_ch, adc_en );
							
							if( ElectronEnergy < 5.0 || adc_en <= 1 ) continue;
							
							mb_evts[GammaCtr]->SetCorGamGen( ElectronEnergy );
							mb_evts[GammaCtr]->SetCorGamCluid( 8 );
							mb_evts[GammaCtr]->SetCorGamCid( adc_num+1 );
							mb_evts[GammaCtr]->SetCorGamSid( adc_ch );
							mb_evts[GammaCtr]->SetCorGamGtd( adc_t - dgf_t );
							mb_evts[GammaCtr]->SetCorGamTheta( 0. );
							mb_evts[GammaCtr]->SetCorGamPhi( 0. );

						}
						
					} // k
					
				} // End search for correlated electrons

				// Count the gamma!
				GammaCtr++;
				
			} // real event in the core?
			
			// Reset
			MaxSegEnergy = -99.;
			MaxSegClu = -1;
			MaxSegCore = -1;
			MaxSegId = -1;
			PartCtr = 0;
			
		} // j
		
		// Electron-particle coincidences
		for( j = 0; j < event->NumberOfAdcs(); j++ ) {
			
			for( k = 0; k < event->Adc(j)->SubEvent()->Size(); k++ ) {
				
				adc_num = event->Adc(j)->ModuleNumber();
				adc_ch  = event->Adc(j)->SubEvent()->AdcChannel(k);
				adc_en  = event->Adc(j)->SubEvent()->AdcValue(k) ;
				adc_t   = event->Adc(j)->Time();

				electron_flag = false;
					
				if( adc_num == 4 && Spede ) { // spede
					
					ElectronEnergy = Cal->AdcEnergy( 4, adc_ch, adc_en );
					electron_flag = true;
					
					mb_evts[GammaCtr]->SetGen( ElectronEnergy );
					mb_evts[GammaCtr]->SetCluid( 8 );
					mb_evts[GammaCtr]->SetCid( 0 );
					mb_evts[GammaCtr]->SetSid( adc_ch );

				}
					
				else if( adc_num >= 0 && adc_num < 4 && adc_ch == 31 && cdpad ) { // pad!

					ElectronEnergy = Cal->AdcEnergy( adc_num, adc_ch, adc_en );
					electron_flag = true;
					
					mb_evts[GammaCtr]->SetGen( ElectronEnergy );
					mb_evts[GammaCtr]->SetCluid( 8 );
					mb_evts[GammaCtr]->SetCid( adc_num+1 );
					mb_evts[GammaCtr]->SetSid( 0 );

				}

				if( electron_flag ) { // Do particles 
					
					for( l = 0; l < Ener_front.size(); l++ ) {
						
						tdiffPG = time[l] - ( adc_t + dtAdc[(int)Quad[l]] );
						
						if( tMinRandomElectron <= tdiffPG && tdiffPG <= tMaxRandomElectron ) coinc_flag = 1;
						else if( tMinPromptElectron <= tdiffPG && tdiffPG <= tMaxPromptElectron ) coinc_flag = 0;
						else coinc_flag = -1;
						
						tdiff_ep->Fill( tdiffPG );
						
						// Add particle
						mb_evts[GammaCtr]->SetPart( Ener_front[l], (int)Chan_front[l], (int)Chan_back[l],
							(double)time[l], (double)event->SuperCycleTime()/1000000, (double)tdiffPG,
							(int)coinc_flag, Quad[l], (int)laser[l], PartCtr );
						
						PartCtr++;
						
					} // End loop All the particles
					
					// Count the electron!
					GammaCtr++;
					
				} // is it in the spede madc?
				
				// Reset
				PartCtr = 0;
				tempStripNum = 0;
				tempRingNum = 0;
				tempStripEnergy = 0;
				tempRingEnergy = 0;
				
			} // k

		} // j
		
		// Progress bar
		if( i%50000 == 0 ) cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries << "% done\r" << flush;

	} // i

	cout << "Check ADCs Fired:" << endl;
	for( i = 0; i < 5; i++ ) cout << " Adc # " << i << " fired: " << CounterAdcCDFired[i] << endl;
   
	outfile->Write();
	outfile->Close();
	delete tr;

	return 0;
	
}

