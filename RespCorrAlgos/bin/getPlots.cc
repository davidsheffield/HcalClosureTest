#include "DijetCalibration/RespCorrAlgos/bin/getPlots.h"

using namespace std;

int main(int argc, char *argv[])
{
    bool isMC;
    TString input_name;
    TString output_name;
    cout << argc << endl;
    if (argc == 4) {
	if (atoi(argv[1]) == 1)
	    isMC = true;
	else if (atoi(argv[1]) == 0)
	    isMC = false;
	else {
	    cout << " Usage: getPlots isMC input output" << endl;
	    return 1;
	}
	input_name = argv[2];
	output_name = argv[3];
    } else {
	cout << " Usage: getPlots isMC input output" << endl;
	return 1;
    }

    cout << "isMC: " << isMC << endl;
    cout << "input: " << input_name << endl;
    cout << "output: " << output_name << endl;

    float maxDeltaEta;
    float minSumJetEt;
    float minJetEt;
    float maxThirdJetEt;
    float maxAlpha;
    int initial_seed;

    TFile *input = new TFile(input_name, "READ");
    TTree *param_tree = 0;
    input->GetObject("Parameters", param_tree);
    param_tree->SetBranchAddress("maxDeltaEta", &maxDeltaEta);
    param_tree->SetBranchAddress("minSumJetEt", &minSumJetEt);
    param_tree->SetBranchAddress("minJetEt", &minJetEt);
    param_tree->SetBranchAddress("maxThirdJetEt", &maxThirdJetEt);
    param_tree->SetBranchAddress("maxAlpha", &maxAlpha);
    param_tree->SetBranchAddress("initial_seed", &initial_seed);
    param_tree->GetEntry(0);

    cout << maxDeltaEta << endl;
    cout << minSumJetEt << endl;
    cout << minJetEt << endl;
    cout << maxThirdJetEt << endl;
    cout << maxAlpha << endl;
    cout << initial_seed << endl;

    DijetRespCorrData data;

    TH1D *h_PassSel = new TH1D("h_PassSelection", "Selection Pass Failures",
			       256, -0.5, 255.5);

    TString inputs[6] =
{"/eos/uscms/store/user/dgsheffi/QCD_Pt_30to50_TuneCUETP8M1_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-100/5c7aa9a575a3633aed507656e4d402e2/tree_*.root",
 "/eos/uscms/store/user/dgsheffi/QCD_Pt_50to80_TuneCUETP8M1_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-100/5c7aa9a575a3633aed507656e4d402e2/tree_*.root",
 "/eos/uscms/store/user/dgsheffi/QCD_Pt_80to120_TuneCUETP8M1_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-100/5c7aa9a575a3633aed507656e4d402e2/tree_*.root",
 "/eos/uscms/store/user/dgsheffi/QCD_Pt_120to170_TuneCUETP8M1_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-100/5c7aa9a575a3633aed507656e4d402e2/tree_*.root",
 "/eos/uscms/store/user/dgsheffi/QCD_Pt_170to300_TuneCUETP8M1_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-100/5c7aa9a575a3633aed507656e4d402e2/tree_*.root",
 "/eos/uscms/store/user/dgsheffi/QCD_Pt_300to470_TuneCUETP8M1_13TeV_pythia8/DijetCalibration_dEta-1p5_sumEt-50_Et-20_3rdEt-100/5c7aa9a575a3633aed507656e4d402e2/tree_*.root"};
    double probability[6] = {1.0, 1.372714e-1, 2.805324e-2, 4.675765e-3,
				 1.175536e-3, 9.256953e-5};
    int seed = initial_seed;
    for (int i=0; i<6; ++i) {
	cout << "Opening " << inputs[i] << endl;
	TChain *tree = new TChain("dijettree");
	tree->Add(inputs[i]);
	DijetTree dijettree(tree, isMC);
	dijettree.SetCuts(maxDeltaEta, minSumJetEt, minJetEt,
			  maxThirdJetEt, maxAlpha);
	dijettree.Loop(&data, h_PassSel, seed, probability[i]);

	++seed;
    }

    cout << data.GetSize() << " data" << endl;

    data.SetDoCandTrackEnergyDiff(false);

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
    TH2D *h_balance_term_vs_weight = new TH2D("tmp", "tmp", 10, 0.0, 10.0,
					      10, 0.0, 10.0);
    data.GetPlots(h_respcorr_init, h_balance, h_Eratio_vs_Eta,
 		  h_balance_term_vs_weight);

     TFile *fout = new TFile(output_name, "RECREATE");
     fout->cd();
     h_balance->Write();
     h_Eratio_vs_Eta->Write();
     fout->Close();

     return 0;
}
