#include "EventBuilder.hh"
#include "stdlib.h"

using namespace std;

ClassImp(GlobalSettings)
ClassImp(UnpackedEvent)
ClassImp(BuiltEvent)
ClassImp(EventBuffer)
ClassImp(EventBuilder)

EventBuilder::EventBuilder() {

	//cout<<__PRETTY_FUNCTION__<<endl;
	Settings = NULL;

	OnBeamFile = NULL;
	OnBeamTree = NULL;
 	OnBeamEvent = NULL;

	OnBeamBackgroundFile = NULL;
	OnBeamBackgroundTree = NULL;
	OnBeamBackgroundEvent = NULL;

	OffBeamFile = NULL;
	OffBeamTree = NULL;
	OffBeamEvent = NULL;

	unpackedEvent = NULL;

	eventBuffer = NULL;

	TotalNumberOfBytesCommittedToOnBeamTree = 0;
	TotalNumberOfBytesCommittedToOnBeamBackgroundTree = 0;
	TotalNumberOfBytesCommittedToOffBeamTree = 0;
 	TotalNumberOfBytesCommittedToScalerTree = 0;
	NumberOfBytesWrittenToOnBeamFile = 0;
	NumberOfBytesWrittenToOnBeamBackgroundFile = 0;
	NumberOfBytesWrittenToOffBeamFile = 0;
	NumberOfBytesWrittenToScalerFile = 0;
	PatternUnitMismatches = 0;

	// Ebis, T1 and SuperCycle pulses
	TotalNumberOfEbisPulses = 0;
	TotalNumberOfT1Pulses = 0;
	TotalNumberOfSuperCyclePulses = 0;

	EbisTime = 0;

	T1Time = 0;

	SuperCycleTime = 0;

	LaserOn = false;
  
	FieldUp = false;
	FieldDown = false;

}

EventBuilder::EventBuilder(GlobalSettings* settings) 
	:Settings(settings) {

	//cout<<__PRETTY_FUNCTION__<<endl;
	if( Settings->VerboseLevel() > 0 ) {

		cout << "opening file " << Settings->OnBeamFile() << endl;

	}

	// open on beam file
	OnBeamFile = new TFile( Settings->OnBeamFile(), "recreate" );

	if( OnBeamFile == NULL ) {

		cerr << __PRETTY_FUNCTION__ << ": Error, couldn't recreate output file ";
		cerr << Settings->OnBeamFile() << ", exiting ... " << endl;
		exit(1);

	}

	if( Settings->VerboseLevel() > 0 ) cout << "creating tree" << endl;

	//create on beam tree
	OnBeamTree = new TTree( "tr", "Tree for on beam data of Coulex setup@Miniball" );

	if( OnBeamTree == NULL ) {

		cerr << __PRETTY_FUNCTION__ << ": Error, couldn't create tree, exiting ... " << endl;
		exit(2);

	}

	if( Settings->VerboseLevel() > 0 )
		cout << "attaching branch to tree " << OnBeamTree->GetName() << ", " << OnBeamTree->GetTitle() << endl;
 
	//attach branch to on beam tree
	OnBeamEvent = new BuiltEvent();
	OnBeamTree->Branch("Event","BuiltEvent",&OnBeamEvent);

	if( Settings->IncludeScaler() ) {

		//attach branch to off beam tree
		fScaler = new SISScaler(); 
		OnBeamTree->Branch("Scaler","SISScaler",&fScaler);

		fDgfScaler = new DgfScaler();
		OnBeamTree->Branch("DgfScaler","DgfScaler",&fDgfScaler);

		fBraggChamber = new BraggChamber();
		OnBeamTree->Branch("BraggChamber","BraggChamber",&fBraggChamber);

    }
  
	OnBeamTree->BranchRef();

	TotalNumberOfBytesCommittedToOnBeamTree = 0;

	if( Settings->VerboseLevel() > 0 )
		cout << "opening file " << Settings->OnBeamBackgroundFile() << endl;

	// open on beam background file
	if( !Settings->SourceRun() ) {

		OnBeamBackgroundFile = new TFile( Settings->OnBeamBackgroundFile(), "recreate" );
      
		if( OnBeamBackgroundFile == NULL ) {

			cerr << __PRETTY_FUNCTION__ << ": Error, couldn't recreate output file ";
			cerr << Settings->OnBeamBackgroundFile() << ", exiting ... " << endl;
			exit(1);

		}
      
		if( Settings->VerboseLevel() > 0 ) cout << "creating tree" << endl;
     
		// create on beam background tree
		OnBeamBackgroundTree = new TTree("bg","Tree for on beam background data of Coulex setup@Miniball");
      
		if( OnBeamBackgroundTree == NULL ) {

			cerr << __PRETTY_FUNCTION__ << ": Error, couldn't create tree, exiting ... " << endl;
			exit(2);

		}
      
		// attach branch to on beam background tree
		OnBeamBackgroundEvent = new BuiltEvent();
		OnBeamBackgroundTree->Branch("Event","BuiltEvent",&OnBeamBackgroundEvent);
      
		if( Settings->IncludeScaler() ) {

			// attach branch to off beam tree
			fScaler = new SISScaler();
			OnBeamBackgroundTree->Branch("Scaler","SISScaler",&fScaler);
			//OnBeamBackgroundTree->Branch("DgfScaler","DgfScaler",&fDgfScaler);

			fDgfScaler = new DgfScaler();
			OnBeamBackgroundTree->Branch("DgfScaler","DgfScaler",&fDgfScaler);
      
			fBraggChamber = new BraggChamber();
			OnBeamBackgroundTree->Branch("BraggChamber","BraggChamber",&fBraggChamber);

		}

		OnBeamBackgroundTree->BranchRef();
      
		TotalNumberOfBytesCommittedToOnBeamBackgroundTree = 0;
      
		// open off beam file
		if( Settings->VerboseLevel() > 0 )
			cout << "opening file " << Settings->OffBeamFile() << endl;
      
		OffBeamFile = new TFile(Settings->OffBeamFile(),"recreate");
      
		if( OffBeamFile == NULL ) {

			cerr << __PRETTY_FUNCTION__ << ": Error, couldn't recreate output file ";
			cerr << Settings->OffBeamFile() << ", exiting ... " << endl;
			exit(1);

		}
      
		// create off beam tree
		if( Settings->VerboseLevel() > 0 ) cout << "creating tree" << endl;
      
		OffBeamTree = new TTree("tr","Tree for off beam data of Coulex setup@Miniball");
      
		if( OffBeamTree == NULL ) {

			cerr<<__PRETTY_FUNCTION__<<": Error, couldn't create tree, exiting ... "<<endl;
			exit(2);

		}
      
		if( Settings->VerboseLevel() > 0 ) {

			cout << "attaching branch to tree " << OffBeamTree->GetName() << ", ";
			cout << OffBeamTree->GetTitle() << endl;

		}
      
		// attach branch to off beam tree
		OffBeamEvent = new BuiltEvent();
		OffBeamTree->Branch("Event","BuiltEvent",&OffBeamEvent);
      
		if( Settings->IncludeScaler() ) {

			// attach branch to off beam tree
			fScaler = new SISScaler();
			OffBeamTree->Branch("Scaler","SISScaler",&fScaler);

			fDgfScaler = new DgfScaler();
			OffBeamTree->Branch("DgfScaler","DgfScaler",&fDgfScaler);

			fBraggChamber = new BraggChamber();
			OffBeamTree->Branch("BraggChamber","BraggChamber",&fBraggChamber);

		}
  
		OffBeamTree->BranchRef();
      
		TotalNumberOfBytesCommittedToOffBeamTree = 0;

    } // if(Settings->SourceRun())
  
	// open scaler file
	if( !Settings->IncludeScaler() ) {

		if( Settings->VerboseLevel() > 0 )
			cout << "opening file " << Settings->ScalerFile() << endl;
     
		ScalerFile = new TFile(Settings->ScalerFile(),"recreate");

		if( ScalerFile == NULL ) {

			cerr << __PRETTY_FUNCTION__ << ": Error, couldn't recreate output file ";
			cerr << Settings->ScalerFile() << ", exiting ... " << endl;
			exit(1);

		}

		// create scaler tree  
		if( Settings->VerboseLevel() > 0 )
			cout << "creating tree" << endl;
     
		ScalerTree = new TTree("sc","Tree for scaler data of Coulex setup@Miniball");
      
		if( ScalerTree == NULL ) {

			cerr << __PRETTY_FUNCTION__ << ": Error, couldn't create tree, exiting ... " << endl;
			exit(2);

		}
      
		if( Settings->VerboseLevel() > 0 ) {

			cout << "attaching branch to tree " << ScalerTree->GetName() << ", ";
			cout << ScalerTree->GetTitle() << endl;

		}

		// attach branch to off beam tree
		fScaler = new SISScaler();
		ScalerTree->Branch("Scaler","SISScaler",&fScaler);
		fDgfScaler = new DgfScaler();
		ScalerTree->Branch("DgfScaler","DgfScaler",&fDgfScaler);
		fBraggChamber = new BraggChamber();
		ScalerTree->Branch("BraggChamber","BraggChamber",&fBraggChamber);
      
		ScalerTree->BranchRef();
      
		TotalNumberOfBytesCommittedToScalerTree = 0;

	} // if( !Settings->IncludeScaler() )

	// general stuff
	unpackedEvent = new UnpackedEvent(Settings);

	eventBuffer = new EventBuffer(Settings);

	TotalNumberOfEbisPulses = 0;
	TotalNumberOfT1Pulses = 0;
	TotalNumberOfSuperCyclePulses = 0;
	PatternUnitMismatches = 0;

	EbisTime = 0;

	T1Time = 0;

	SuperCycleTime = 0;

	LaserOn = false;
  
	//Field = false;
	FieldUp = false;
 	FieldDown = false;

}

EventBuilder::~EventBuilder() {
	
	//cout<<__PRETTY_FUNCTION__<<endl;
	if( Settings != NULL )
		delete Settings;

	if( unpackedEvent != NULL )
		delete unpackedEvent;

	if( eventBuffer != NULL )
		delete eventBuffer;

}

int EventBuilder::TrashEvent() {

	if( Settings->VerboseLevel() > 1 )
		cout << endl << "start of " << __PRETTY_FUNCTION__ << endl;

	if( Settings->VerboseLevel() > 3 )
		cout << "clearing unpackedEvent = " << hex << unpackedEvent << dec << endl;

	unpackedEvent->ClearEvt();

	return 0;

}

int EventBuilder::ProcessEvent( const MBSDataIO *mbs ) {

	if( Settings->VerboseLevel() > 1 )
		cout << endl << "start of " << __PRETTY_FUNCTION__ << endl;

	if( Settings->VerboseLevel() > 3 )
		cout << "clearing unpackedEvent = " << hex << unpackedEvent << dec << endl;


	unpackedEvent->ClearEvt();

	// unpack the event
	int status = unpackedEvent->ProcessEvent(mbs);
	if( status != 0 ) {

		if( Settings->VerboseLevel() > 2 ) 
			cout << "processing of event didn't work!" << endl;

		return status;

	}

	else if( Settings->VerboseLevel() > 3 ) {

		cout << "unpacking of event worked!" << endl;

	}

	// verify the event (unless the new mesytec adcs are used)
	if( !Settings->MesytecAdc() ) {

		if( !unpackedEvent->Verify() ) {

			if( Settings->VerboseLevel() > 2 )
				cout << "verifying of event didn't work!" << endl;

			return 3;

		}

		else if( Settings->VerboseLevel() > 3 )
			cout << "verification of event worked!" << endl;

	}

	BuildEvent();

	return 0;

}

void EventBuilder::BuildEvent() {

	if( Settings->VerboseLevel() > 1 )
		cout << endl << "start of " << __PRETTY_FUNCTION__ << endl;

	unsigned int tse;
	int ts, module;
	size_t VmeSubEvent;
	vector<unsigned short> NumberOfTimestamps(Settings->NumberOfTimestampModules());
	int SumOfTimestamps;
	int NumberOfEbisPulses;
	bool EbisOn = false;

	bool* AllBadTimestamp = NULL;
	int* NumberOfBadTimestamp = NULL;
	if( !Settings->MesytecAdc() ) {

		AllBadTimestamp = new bool[Settings->NumberOfTimestampModules()];
		NumberOfBadTimestamp = new int[Settings->NumberOfTimestampModules()];

		for( ts = 0; ts < Settings->NumberOfTimestampModules(); ts++ ) {

			AllBadTimestamp[ts] = false;
			NumberOfBadTimestamp[ts] = 0;

		}

	}

	// clear built events
	eventBuffer->ClearEvt();

	if( !Settings->MesytecAdc() ) {

		SumOfTimestamps = unpackedEvent->GetTimestamps(NumberOfTimestamps);
		if( Settings->VerboseLevel() > 1 )  {

			cout << endl << "read out event nr. " << unpackedEvent->GetEventNumber()-1;
			cout << ": total number of timestamps in " << Settings->NumberOfTimestampModules();
			cout << " (= 'NumberOfTimestampModules') modules: " << SumOfTimestamps;
			cout << " (= 'SumOfTimestamps')" << endl;

			for( ts=0; ts < Settings->NumberOfTimestampModules(); ts++ ) {

				cout << "number of timestamps in timestamp module nr. " << ts << ":  ";
				cout << NumberOfTimestamps[ts] << " (= 'NumberOfTimestamps[" << ts;
				cout << "]')" << endl;

			}

		}

    }

	NumberOfEbisPulses = 0;

	// loop over events in 'EbisT1AndSuperCycleTimestampModule' 
	for( tse = 0; tse < unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetNumberOfSubEvents(); tse++ ) { 

		if( Settings->VerboseLevel() > 1 ) {

			cout << __PRETTY_FUNCTION__ << ": read event nr. ";
			cout << unpackedEvent->GetEventNumber()-1 << ": hitpattern (0x" << hex;
			cout << unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern();
			cout << dec << " = unpackedEvent->GetDgfModule(" << Settings->EbisT1AndSuperCycleModule();
			cout << ")->GetSubEvent(" << tse << ")->GetHitPattern())!" << endl;

		}

		// EBIS pulse                             
		// if EBIS pulse hitpattern -> count them 
		if( unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern() & (1<<Settings->EbisChannel()) ) {

			// count number of EBIS hitpatterns 
			NumberOfEbisPulses++;
	  
			EbisTime = unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetLongFastTriggerTime(Settings->EbisChannel()); 
			TotalNumberOfEbisPulses++;
	  
			if( Settings->VerboseLevel() > 1 ) {

				cout << __PRETTY_FUNCTION__ << ": read event nr. ";
				cout << unpackedEvent->GetEventNumber()-1 << ": found EBIS hitpattern (0x";
				cout << hex << unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern();
				cout << dec << " = unpackedEvent->GetDgfModule(";
				cout << Settings->EbisT1AndSuperCycleModule() << ")->GetSubEvent(";
				cout << tse << ")->GetHitPattern())!" << endl;

			}

		}
      
      
		// T1 hitpattern 
		if( unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern() & (1<<Settings->T1Channel()) ) {

			T1Time = unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetLongFastTriggerTime(Settings->T1Channel());
			TotalNumberOfT1Pulses++;
	  
			if( Settings->VerboseLevel() > 1 ) {

				cout << __PRETTY_FUNCTION__ << ": read event nr. " << unpackedEvent->GetEventNumber()-1;
				cout << ": found T1 hitpattern (0x" << hex;
				cout << unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern();
				cout << dec << ")!" << endl;

			}

		}
 
		// Super Cycle hitpattern
		if( unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern() & (1<<Settings->SuperCycleChannel()) ) {

			SuperCycleTime = unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetLongFastTriggerTime(Settings->SuperCycleChannel());
			TotalNumberOfSuperCyclePulses++;
	  
			if( Settings->VerboseLevel() > 1 ) {

				cout << __PRETTY_FUNCTION__ << ": read event nr. ";
				cout << unpackedEvent->GetEventNumber()-1 << ": found SuperCycle hitpattern (0x";
				cout << hex << unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->GetSubEvent(tse)->GetHitPattern();
				cout << dec << ")!" << endl;

			}

		}

	} // for(tse=0;tse<unpackedEvent->GetDgfModule(Settings->EbisT1AndSuperCycleModule())->numofevents;tse++) 

	if( Settings->VerboseLevel() > 1 )
		cout << "NumberOfEbisPulses = " << NumberOfEbisPulses << endl;
 
	// check EBIS channel, set marker_flag[]'s: '1': ON-, '2': OFF-BEAM 
	if( NumberOfEbisPulses == 1 ) EbisOn = true;
	else if( NumberOfEbisPulses > 1 ) {

		cout << __PRETTY_FUNCTION__ << ": read event nr. ";
		cout << unpackedEvent->GetEventNumber()-1 << ": found " << NumberOfEbisPulses;
		cout << " EBIS pulse hitpatterns instead of 1 -> set event to _OFF_ beam event!" << endl;

	}
    
	if( !Settings->MesytecAdc() ) {

		// loop over Settings->NumberOfTimestampModules() to check for timestamp mismatches 
		for( ts=0; ts < Settings->NumberOfTimestampModules(); ts++ ) {

			// if at least ONE timestamp (i.e. VME gate hitpattern) found in one of Settings->NumberOfTimestampModules():      
			// check eventnumbers and numofevents in CAEN V785 modules
	  
			// check if at least one timestamp in actual timestamp module 
			if( NumberOfTimestamps[ts] > 0 ) {

				// set flag that indicates that there was at least one timestamp 
				for( module = ts*Settings->NumberOfAdcsPerTimestampModule(); module < (ts+1)*Settings->NumberOfAdcsPerTimestampModule(); module++ ) {

					if( unpackedEvent->GetAdcModule(module)->GetNumberOfSubEvents() != (unsigned int) NumberOfTimestamps[ts] ) {

						//BadTimestamp[ts][module] = true;
						AllBadTimestamp[ts] = true;
						NumberOfBadTimestamp[ts]++;

						if( Settings->VerboseLevel() > 3 ) {

							cout << __PRETTY_FUNCTION__ << ": read event nr. " << unpackedEvent->GetEventNumber()-1;
							cout << ": ts mod. " << ts << " (ana-mod-nr: " << Settings->TimestampModule(ts);
							cout << "): mismatch of NumberOfTimestamps[" << ts << "] (= " << NumberOfTimestamps[ts];
							cout << ") and subevents in CAEN module " << module << " (";
							cout << unpackedEvent->GetAdcModule(module)->GetNumberOfSubEvents();
							cout << "), # of bad ts = " << NumberOfBadTimestamp[ts] << endl;

						}

					}

					else {

						if( Settings->VerboseLevel() > 3 ) {

				  			cout<<__PRETTY_FUNCTION__<<": read event nr. "<<unpackedEvent->GetEventNumber()-1<<": ts mod. "<<ts<<" (ana-mod-nr: "<<Settings->TimestampModule(ts)<<"): match of NumberOfTimestamps["<<ts<<"] (= "<<NumberOfTimestamps[ts]<<") and subevents in CAEN module "<<module<<" ("<<unpackedEvent->GetAdcModule(module)->GetNumberOfSubEvents()<<"), # of bad ts = "<<NumberOfBadTimestamp[ts]<<endl;

						}

					}

				}

			}

			else if( Settings->VerboseLevel() > 3 ) {

				cout << "no timestamps in " << ts << ". module " << Settings->TimestampModule(ts);
				cout << ", AllBadTimestamp[ts] = " << AllBadTimestamp[ts] << endl;

			}
		  
			// report error if mismatch of NumberOfTimestamps and number of subevents found in VME channel 
			if( AllBadTimestamp[ts] ) {

				cout << endl << __PRETTY_FUNCTION__ << ": read event nr. " << unpackedEvent->GetEventNumber()-1;
				cout << ": ts mod. " << ts << " (ana-mod-nr: " << Settings->TimestampModule(ts);
				cout << "): mismatch of NumberOfTimestamps[" << ts << "] (= " << NumberOfTimestamps[ts];
				cout << ") and subevents in " << NumberOfBadTimestamp[ts] << " CAEN modules (of VME modules ";
				cout << ts*Settings->NumberOfAdcsPerTimestampModule() << " - ";
				cout << (ts+1)*Settings->NumberOfAdcsPerTimestampModule() << ")" << endl;

			}

		} // for(ts=0; ts < Settings->NumberOfTimestampModules(); ts++) 

	} //  if(!Settings->MesytecAdc())
  
	if( unpackedEvent->GetPatternUnit(0)->LaserOn() ) LaserOn = true;
	else LaserOn = false;
  
  
	// ========================== 
	// START of the EVENTBUILDING 
	// ========================== 


	// add particles to event buffer
	if( !Settings->MesytecAdc() ) {

		// loop over Settings->NumberOfTimestampModules() 
		for( ts=0; ts < Settings->NumberOfTimestampModules(); ts++ ) { 

			// check if timestamping of VME ADC channels is OK
			if( AllBadTimestamp[ts] ) {

				cout << __PRETTY_FUNCTION__ << ": read event nr. " << unpackedEvent->GetEventNumber()-1;
				cout << ": timestamping error in " << NumberOfBadTimestamp[ts];
				cout << " CAEN V7X5 modules -> NO ADC channels are processed!" << endl;

			}

			else {   
	   
				// loop over adcs
				for( int adc = ts*Settings->NumberOfAdcsPerTimestampModule(); adc < (ts+1)*Settings->NumberOfAdcsPerTimestampModule(); adc++ ) {

					// loop over subevents in ADC buffers which are then the first builtevents 
					for( VmeSubEvent=0; VmeSubEvent < NumberOfTimestamps[ts]; VmeSubEvent++ ) {

						eventBuffer->AddParticle(unpackedEvent->GetAdcModule(adc)->GetModuleNumber(),unpackedEvent->GetAdcModule(adc)->GetSubEvent(VmeSubEvent),unpackedEvent->GetTimestampModule(ts)->GetSubEvent(VmeSubEvent)->GetLongFastTriggerTime(Settings->TimestampChannel()), LaserOn, unpackedEvent->GetPatternUnit(0)->FieldUp(), unpackedEvent->GetPatternUnit(0)->FieldDown());

			  	  }

				}

			}

		}

    }

	else {

		for( int adc = 0; adc < Settings->NumberOfAdcModules(); adc++ ) {

		// loop over subevents in ADC buffers which are then the first builtevents 

			if( adc<Settings->NumberOfAdcModules()-Settings->NofCaenAdc() ) {

				for( VmeSubEvent = 0; VmeSubEvent < unpackedEvent->GetAdcModule(adc)->GetNumberOfSubEvents(); VmeSubEvent++ ) {

					eventBuffer->AddParticle(unpackedEvent->GetAdcModule(adc)->GetModuleNumber(), unpackedEvent->GetAdcModule(adc)->GetSubEvent(VmeSubEvent), LaserOn, unpackedEvent->GetPatternUnit(0)->FieldUp(), unpackedEvent->GetPatternUnit(0)->FieldDown());

				}

			}

		}

	}
  
	// add gammas to event buffer
	for( int dgf = 0; dgf < unpackedEvent->GetNumberOfDgfModules(); dgf++ ) {

		for( size_t SubEvent = 0; SubEvent < unpackedEvent->GetDgfModule(dgf)->GetNumberOfSubEvents(); SubEvent++ ) {

				eventBuffer->AddGamma(unpackedEvent->GetDgfModule(dgf)->GetModuleNumber(), unpackedEvent->GetDgfModule(dgf)->GetSubEvent(SubEvent));

		}

    }

	// add Ebis, T1 and SuperCycleTimes!
	eventBuffer->AddTiming(EbisTime, T1Time, SuperCycleTime);

	// set event number
	eventBuffer->EventNumber(unpackedEvent->GetEventNumber()-1);

	// sort event buffer after time
	eventBuffer->Sort();

	// write eventBuffer to file
	int NumberOfBytesCommitted;

	// check whether on or off beam readout
	if( EbisOn || Settings->SourceRun() ) {

		// loop over all event in the buffer and fill them into the tree
		for( size_t i = 0; i < eventBuffer->NumberOfBuiltEvents(); i++ ) {

			// check whether event is in the user-defined ebis window
			if( (Settings->EbisWindowLowerEdge() < eventBuffer->GetSortedEvent(i)->GetTime() - EbisTime &&
				eventBuffer->GetSortedEvent(i)->GetTime() - EbisTime < Settings->EbisWindowUpperEdge()) ||
				Settings->SourceRun()) {

				OnBeamEvent = eventBuffer->GetSortedEvent(i);

				// if the scaler data is to be included and if there actually is scaler data
				if( Settings->IncludeScaler() && unpackedEvent->ScalerData() ) {

					fScaler = unpackedEvent->GetScaler();
					fDgfScaler = unpackedEvent->GetDgfScaler();
					fBraggChamber = unpackedEvent->GetBraggChamber();

				}

				NumberOfBytesCommitted = OnBeamTree->Fill();
	      
				if( Settings->VerboseLevel() > 1 ) {

					cout << "filled " << NumberOfBytesCommitted << " bytes into on-beam tree";
					cout << OnBeamTree->GetName() << ", " << OnBeamTree->GetTitle() << endl;

				}
	      
				if( NumberOfBytesCommitted == 0 ) {

					cerr << __PRETTY_FUNCTION__ << ": warning, all branches of tree " << OnBeamTree->GetName();
					cerr << ", " << OnBeamTree->GetTitle() << " are disabled!!!" << endl;

				}

				if( NumberOfBytesCommitted == -1 ) {

					cerr << __PRETTY_FUNCTION__ << ": error on commiting branches to tree ";
					cerr << OnBeamTree->GetName() << ", " << OnBeamTree->GetTitle() << ", exiting now!" << endl;
					exit(1);

				}
	      
				TotalNumberOfBytesCommittedToOnBeamTree += NumberOfBytesCommitted;

			}

			else {

				OnBeamBackgroundEvent = eventBuffer->GetSortedEvent(i);
	      
				NumberOfBytesCommitted = OnBeamBackgroundTree->Fill();
	      
				if( Settings->VerboseLevel() > 1 ) {

					cout << "filled " << NumberOfBytesCommitted << " bytes into on-beam tree";
					cout << OnBeamBackgroundTree->GetName() << ", " << OnBeamBackgroundTree->GetTitle() << endl;

				}
	      
				if( NumberOfBytesCommitted == 0 ) {

					cerr << __PRETTY_FUNCTION__ << ": warning, all branches of tree ";
					cerr << OnBeamBackgroundTree->GetName() << ", " << OnBeamBackgroundTree->GetTitle();
					cerr << " are disabled!!!" << endl;

				}

				if( NumberOfBytesCommitted == -1 ) {

					cerr << __PRETTY_FUNCTION__ << ": error on commiting branches to tree ";
					cerr << OnBeamBackgroundTree->GetName() << ", " << OnBeamBackgroundTree->GetTitle();
					cerr << ", exiting now!" << endl;
					exit(1);

				}
	      
				TotalNumberOfBytesCommittedToOnBeamBackgroundTree += NumberOfBytesCommitted;

			}

		}

	}

	else {

		// loop over all event in the buffer and fill them into the tree
		for( size_t i = 0; i < eventBuffer->NumberOfBuiltEvents(); i++ ) {

			OffBeamEvent = eventBuffer->GetSortedEvent(i);

			NumberOfBytesCommitted = OffBeamTree->Fill();

			if( Settings->VerboseLevel() > 1 ) {

				cout << "filled " << NumberOfBytesCommitted << " bytes into off-beam tree ";
				cout << OffBeamTree->GetName() << ", " << OffBeamTree->GetTitle() << endl;

			}
	  
			if( NumberOfBytesCommitted == 0 ) {

				cerr << __PRETTY_FUNCTION__ << ": warning, all branches of tree ";
				cerr << OffBeamTree->GetName() << ", " << OffBeamTree->GetTitle();
				cerr << " are disabled!!!" << endl;

			}

			if( NumberOfBytesCommitted == -1 ) {

				cerr << __PRETTY_FUNCTION__ << ": error on commiting branches to tree ";
				cerr << OffBeamTree->GetName() << ", " << OffBeamTree->GetTitle();
				cerr << ", exiting now!" << endl;
				exit(1);

			}

			TotalNumberOfBytesCommittedToOffBeamTree += NumberOfBytesCommitted;

		}

	} // else of EbisOn

	// ========================== 
	// START of the EVENTBUILDING 
	// ========================== 
  
	if( unpackedEvent->ScalerData() ) {

		fScaler = unpackedEvent->GetScaler();
		fDgfScaler = unpackedEvent->GetDgfScaler();
		fBraggChamber = unpackedEvent->GetBraggChamber();

		if( !Settings->IncludeScaler() ) {

			NumberOfBytesCommitted = ScalerTree->Fill();
	  
			if( Settings->VerboseLevel() > 1 ) {

				cout << "filled " << NumberOfBytesCommitted << " bytes into off-beam tree ";
				cout << ScalerTree->GetName() << ", " << ScalerTree->GetTitle() << endl;

			}
	  
			if( NumberOfBytesCommitted == 0 ) {

				cerr << __PRETTY_FUNCTION__ << ": warning, all branches of tree ";
				cerr << ScalerTree->GetName() << ", " << ScalerTree->GetTitle() << " are disabled!!!" << endl;

			}

			if( NumberOfBytesCommitted == -1 ) {

				cerr << __PRETTY_FUNCTION__ << ": error on commiting branches to tree ";
				cerr << ScalerTree->GetName() << ", " << ScalerTree->GetTitle() << ", exiting now!" << endl;
				exit(1);

			}
	  
			TotalNumberOfBytesCommittedToScalerTree += NumberOfBytesCommitted;

		} // if(!Settings->IncludeScaler())

	}
  
} // BuildEvent

void EventBuilder::Finish() {

	Statistics();

	long double i;

	if( Settings->VerboseLevel() > 1 )
		cout << endl << "start of " << __PRETTY_FUNCTION__ << endl;

	// returns number of bytes written, 0 = unable to write tree
	OnBeamFile->cd();

	if( Settings->VerboseLevel() > 0 ) {

		cout << "writing on-beam tree " << OnBeamTree->GetName();
		cout << ", " << OnBeamTree->GetTitle() << endl;

	}

	NumberOfBytesWrittenToOnBeamFile = OnBeamTree->Write("",TObject::kOverwrite);
  
	if( !Settings->SourceRun() ) {

		// returns number of bytes written, 0 = unable to write tree
		OnBeamBackgroundFile->cd();
      
		if( Settings->VerboseLevel() > 0 ) {

			cout << "writing on-beam background tree " << OnBeamBackgroundTree->GetName();
			cout << ", " << OnBeamBackgroundTree->GetTitle() << endl;

		}

		NumberOfBytesWrittenToOnBeamBackgroundFile = OnBeamBackgroundTree->Write("",TObject::kOverwrite);
      
		OffBeamFile->cd();
      
		if( Settings->VerboseLevel() > 0 ) {

			cout << "writing off-beam tree " << OffBeamTree->GetName();
			cout << ", " << OffBeamTree->GetTitle() << endl;

		}

		NumberOfBytesWrittenToOffBeamFile = OffBeamTree->Write("",TObject::kOverwrite);

	}
  
	if( !Settings->IncludeScaler() ) {

		ScalerFile->cd();
      
		if( Settings->VerboseLevel() > 0 ) {

			cout << "writing scaler tree " << ScalerTree->GetName();
			cout << ", " << ScalerTree->GetTitle() << endl;

		}

		NumberOfBytesWrittenToScalerFile = ScalerTree->Write("",TObject::kOverwrite);

	}
  
	if( !Settings->SourceRun() && Settings->VerboseLevel() > 0 ) {

		cout << "closing files " << OnBeamFile->GetName() << ", ";
		cout << OnBeamBackgroundFile->GetName() << ", " << OffBeamFile->GetName();
		cout << " and " << ScalerFile->GetName() << endl;

    }
  
	cout << "Number of ebis pulses:       " << setw(10) << TotalNumberOfEbisPulses << endl;
	cout << "Number of t1 pulses:         " << setw(10) << TotalNumberOfT1Pulses << endl;
	cout << "Number of supercycle pulses: " << setw(10) << TotalNumberOfSuperCyclePulses << endl;
  
	cout << "committed ";

	for( i = 4; i >= 0; i-- ) {

		if( i > 0 && TotalNumberOfBytesCommittedToOnBeamTree/((long long) pow(1000,i)) == 0)
			cout << "    " ;

		else {

			cout << setw(3);
			cout << (TotalNumberOfBytesCommittedToOnBeamTree - ((long long) pow(1000,i+1))* (TotalNumberOfBytesCommittedToOnBeamTree/((long long) pow(1000,i+1))))/((long long) pow(1000,i));
			cout << " ";

		}

	}

	cout << " bytes to tree " << OnBeamTree->GetName() << ", '" << OnBeamTree->GetTitle() << "'" << endl;

	if( !Settings->SourceRun() ) {

		cout << "and       ";
		for(i = 4; i >= 0; i--) {

			if(i > 0 && TotalNumberOfBytesCommittedToOnBeamBackgroundTree/((long long) pow(1000,i)) == 0)
				cout << "    ";

			else {

				cout << setw(3);
				cout << (TotalNumberOfBytesCommittedToOnBeamBackgroundTree - ((long long) pow(1000,i+1))* (TotalNumberOfBytesCommittedToOnBeamBackgroundTree/((long long) pow(1000,i+1))))/((long long) pow(1000,i));
				cout << " ";

			}

		}

		cout << " bytes to tree " << OnBeamBackgroundTree->GetName() << ", '";
		cout << OnBeamBackgroundTree->GetTitle() << "'" << endl;
		cout << "and       ";

		for(i = 4; i >= 0; i--) {

			if( i > 0 && TotalNumberOfBytesCommittedToOffBeamTree/((long long) pow(1000,i)) == 0)
				cout << "    ";

			else {

				cout << setw(3);
				cout << (TotalNumberOfBytesCommittedToOffBeamTree - ((long long) pow(1000,i+1))* (TotalNumberOfBytesCommittedToOffBeamTree/((long long) pow(1000,i+1))))/((long long) pow(1000,i));
				cout << " ";

			}

		}

		cout << " bytes to tree " << OffBeamTree->GetName() << ", '";
		cout << OffBeamTree->GetTitle() << "'" << endl;

    }
  
  cout<<"wrote     ";
  for(i = 4; i >= 0; i--)
    {
      if(i > 0 && NumberOfBytesWrittenToOnBeamFile/((long long) pow(1000,i)) == 0)
	{
	  cout<<"    ";
	}
      else
	{
	  cout<<setw(3)<<(NumberOfBytesWrittenToOnBeamFile - ((long long) pow(1000,i+1))* (NumberOfBytesWrittenToOnBeamFile/((long long) pow(1000,i+1))))/((long long) pow(1000,i))<<" ";
	}
    }
  cout<<" bytes to file "<<Settings->OnBeamFile()<<" \t=> compressed by a factor of "<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<((double)TotalNumberOfBytesCommittedToOnBeamTree)/NumberOfBytesWrittenToOnBeamFile<<endl;
  if(!Settings->SourceRun())
    {
      cout<<",         ";
      for(i = 4; i >= 0; i--)
	{
	  if(i > 0 && NumberOfBytesWrittenToOnBeamBackgroundFile/((long long) pow(1000,i)) == 0)
	    {
	      cout<<"    ";
	    }
	  else
	    {
	      cout<<setw(3)<<(NumberOfBytesWrittenToOnBeamBackgroundFile - ((long long) pow(1000,i+1))* (NumberOfBytesWrittenToOnBeamBackgroundFile/((long long) pow(1000,i+1))))/((long long) pow(1000,i))<<" ";
	    }
	}
      cout<<" bytes to file "<<Settings->OnBeamBackgroundFile()<<" \t=> compressed by a factor of "<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<((double)TotalNumberOfBytesCommittedToOnBeamBackgroundTree)/NumberOfBytesWrittenToOnBeamBackgroundFile<<endl
	  <<",         ";
      for(i = 4; i >= 0; i--)
	{
	  if(i > 0 && NumberOfBytesWrittenToOffBeamFile/((long long) pow(1000,i)) == 0)
	    {
	      cout<<"    ";
	    }
	  else
	    {
	      cout<<setw(3)<<(NumberOfBytesWrittenToOffBeamFile - ((long long) pow(1000,i+1))* (NumberOfBytesWrittenToOffBeamFile/((long long) pow(1000,i+1))))/((long long) pow(1000,i))<<" ";
	    }
	}
      cout<<" bytes to file "<<Settings->OffBeamFile()<<" \t=> compressed by a factor of "<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<((double)TotalNumberOfBytesCommittedToOffBeamTree)/NumberOfBytesWrittenToOffBeamFile<<endl;
    }
  cout<<"and       ";
  for(i = 4; i >= 0; i--)
    {
      if(i > 0 && NumberOfBytesWrittenToScalerFile/((long long) pow(1000,i)) == 0)
	{
	  cout<<"    ";
	}
      else
	{
	  cout<<setw(3)<<(NumberOfBytesWrittenToScalerFile - ((long long) pow(1000,i+1))* (NumberOfBytesWrittenToScalerFile/((long long) pow(1000,i+1))))/((long long) pow(1000,i))<<" ";
	}
    }
  cout<<" bytes to file "<<Settings->ScalerFile()<<" \t=> compressed by a factor of "<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<((double)TotalNumberOfBytesCommittedToScalerTree)/NumberOfBytesWrittenToScalerFile<<endl;

  OnBeamFile->Close();
  if(!Settings->SourceRun())
    {
      OnBeamBackgroundFile->Close();
      OffBeamFile->Close();
    }
  if(!Settings->IncludeScaler())
    ScalerFile->Close();
}

void EventBuilder::Statistics()
{
  unpackedEvent->Statistics();
  cout<<"pattern unit mismatches:         "<<setw(12)<<PatternUnitMismatches<<" ("<<setw(4)<<((int)(1000.*PatternUnitMismatches/unpackedEvent->GetEventNumber()))/10.<<" %)"<<endl<<endl;
}
