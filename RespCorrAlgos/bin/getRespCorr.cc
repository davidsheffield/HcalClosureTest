#include "DijetCalibration/RespCorrAlgos/bin/getRespCorr.h"

using namespace std;

int main(int argc, char *argv[])
{
    bool isMC = false;
    float maxDeltaEta_ = 0.5;
    float minSumJetEt_ = 50.0;
    float minJetEt_ = 25.0;
    float maxThirdJetEt_ = 15.0;

    cout << argc << endl;
    if (argc == 5) {
	if (atoi(argv[1]) == 1) {
	    isMC = true;
	} else if (atoi(argv[1]) == 0) {
	    isMC = false;
	} else {
	    cout << " Usage: getRespCorr isMC dEta leadingEt 3rdEt" <<
		endl;
	    return 1;
	}
	maxDeltaEta_ = atof(argv[2]);
	minSumJetEt_ = atof(argv[3]);
	maxThirdJetEt_ = atof(argv[4]);
    } else {
	cout << "Not right number of arguments." << endl;
	cout << " Usage: getRespCorr isMC dEta sumEt 3rdEt" << endl;
	return 1;
    }

    cout << "isMC: " << isMC << endl;
    cout << "dEta: " << maxDeltaEta_ << endl;
    cout << "sumEt: " << minSumJetEt_ << endl;
    cout << "3rdEt: " << maxThirdJetEt_ << endl;

    TChain *tree = new TChain("dijettree");

    TString input = "/eos/uscms/store/user/dgsheffi/QCD_Pt-15to7000_TuneCUETP8M1_Flat_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-75/afdb1fa6d9e6b5be9d47dda947717251/tree_*.root";
    cout << "Opening file: " << input << endl;
    tree->Add(input);
    cout << "File opened." << endl;

    int decimal = static_cast<int>(maxDeltaEta_*10)
	          - static_cast<int>(maxDeltaEta_)*10;
    //TString output = "/uscms_data/d1/dgsheffi/HCal/corrections/QCD_Pt-120To170_dEta-"+to_string(static_cast<int>(maxDeltaEta_))+"p"+to_string(decimal)+"_Et-"+to_string(static_cast<int>(minJetEt_))+"_3rdEt-"+to_string(static_cast<int>(maxThirdJetEt_))+"_noNeutralPUcorr.root";
    TString output =
	"/uscms_data/d1/dgsheffi/HCal/corrections/QCD_Pt-15to7000_dEta-"
	+ to_string(static_cast<int>(maxDeltaEta_)) + "p" + to_string(decimal)
	+ "_sumEt-" + to_string(static_cast<int>(minSumJetEt_)) + "_3rdEt-"
	+ to_string(static_cast<int>(maxThirdJetEt_)) + "_1fb-1.root";
    //output = "test.root";

    DijetRespCorrData data;

    TH1D *h_PassSel_ = new TH1D("h_PassSelection", "Selection Pass Failures",
				256, -0.5, 255.5);
    TH1D *h_weights = LogXTH1D("h_weights","weights",200,1.0e-12,1.1);
    h_weights->GetXaxis()->SetTitle("weight");
    h_weights->GetYaxis()->SetTitle("events");

    DijetTree dijettree(tree);
    dijettree.SetCutMaxDelta(maxDeltaEta_);
    dijettree.SetCutMinSumJetEt(minSumJetEt_);
    dijettree.SetCutMinJetEt(minJetEt_);
    dijettree.SetCutMaxThirdJetEt(maxThirdJetEt_);
    dijettree.Loop(&data);
//loop

    cout << data.GetSize() << " data" << endl;

    //cout << "Passes: " << nEvents - fails << " Fails: " << fails << endl;
    cout << "Do CandTrack? " << data.GetDoCandTrackEnergyDiff() << endl;
    data.SetDoCandTrackEnergyDiff(false);
    cout << "Do CandTrack? " << data.GetDoCandTrackEnergyDiff() << endl;

    //return 0;

    TH1D *h_respcorr_init = new TH1D("h_respcorr_init",
				     "responce corrections of 1",
				     83, -41.5, 41.5);
    for(int i=1; i<84; ++i){
	h_respcorr_init->SetBinContent(i, 1.0);
    }
    TH1D *h_balance = new TH1D("h_balance", "dijet balance", 200, -2.0, 2.0);
    TH2D *h_Eratio_vs_Eta = new TH2D("h_Eratio_vs_Eta",
				     "E_{reco}/E_{gen} vs. #eta",
				     200, -5.0, 5.0, 200, 0.0, 2.0);
    TH2D *h_balance_term_vs_weight = LogXLogYTH2D("h_balance_term_vs_weight",
						  "B^{2}/(#DeltaB)^{2} vs weight",
						  200, 1.0e-7, 1.01,
						  200, 1.0e-7, 1.0e2);
    data.SetResolution(0.384);
    data.GetPlots(h_respcorr_init, h_balance, h_Eratio_vs_Eta,
		  h_balance_term_vs_weight);
    data.SetResolution(h_balance);

    TH1D *hist = data.doFit("h_corr", "Response Corrections");
    hist->GetXaxis()->SetTitle("i_{#eta}");
    hist->GetYaxis()->SetTitle("response corrections");

    TFile *fout = new TFile(output, "RECREATE");
    fout->cd();
    hist->Write();
    h_PassSel_->Write();
    h_weights->Write();
    h_balance->Write();
    h_Eratio_vs_Eta->Write();
    h_balance_term_vs_weight->Write();
    fout->Close();

    //cout << "Passes: " << nEvents - fails << " Fails: " << fails << endl;
    cout << "Events that passed cuts: " << h_PassSel_->GetBinContent(1) << endl;

    return 0;
}

TH1D* LogXTH1D(const char* name, const char* title, Int_t nbinsx,
	       Double_t xlow, Double_t xup)
{
    Double_t logxlow = TMath::Log10(xlow);
    Double_t logxup = TMath::Log10(xup);
    Double_t binwidth = (logxup - logxlow)/nbinsx;
    Double_t xbins[nbinsx+1];
    for (int i=0; i<=nbinsx; ++i) {
	xbins[i] = TMath::Power(10,logxlow + i*binwidth);
    }
    TH1D *histogram = new TH1D(name, title, nbinsx, xbins);
    return histogram;
}

TH2D *LogXLogYTH2D(const char* name, const char* title,
		   Int_t nbinsx, Double_t xlow, Double_t xup,
		   Int_t nbinsy, Double_t ylow, Double_t yup)
{
    Double_t logxlow = TMath::Log10(xlow);
    Double_t logxup = TMath::Log10(xup);
    Double_t xbinwidth = (logxup - logxlow)/nbinsx;
    Double_t xbins[nbinsx+1];
    for (int i=0; i<=nbinsx; ++i) {
	xbins[i] = TMath::Power(10,logxlow + i*xbinwidth);
    }
    Double_t logylow = TMath::Log10(ylow);
    Double_t logyup = TMath::Log10(yup);
    Double_t ybinwidth = (logyup - logylow)/nbinsy;
    Double_t ybins[nbinsy+1];
    for (int i=0; i<=nbinsy; ++i) {
	ybins[i] = TMath::Power(10,logylow + i*ybinwidth);
    }
    TH2D *histogram = new TH2D(name, title, nbinsx, xbins, nbinsy, ybins);
    return histogram;
}