// Includes from PrEW
#include "GlobalVar/Chiral.h"
#include "Output/Printer.h"

// Includes from PrEWUtils
#include "DataHelp/BinSelector.h"
#include "Runners/ParallelRunner.h"
#include "Setups/RKDistrSetup.h"

#include "spdlog/spdlog.h"

int main (int /*argc*/, char **/*argv*/) {
  spdlog::set_level(spdlog::level::info);
  
  int energy = 250;
  int n_threads = 3;
  int n_toys = 10;
  std::string minuit_minimizers = "Combined(1000000,1000000,0.00001)";
  std::string prew_minimizer = "PoissonNLL";
  std::string output_path = "../output/fit_results.out";
  
  // Minimum SM-prediction a bin must have to be considered
  // double bin_cutoff_value = 10; 
    
  spdlog::info("Start test.");
  
  spdlog::info("Create setup.");
  PrEWUtils::Setups::RKDistrSetup setup {};
  
  spdlog::info("Add files and energies.");
  setup.add_input_file("/afs/desy.de/group/flc/pool/beyerjac/TGCAnalysis/PrEW/testdata/RK_examplefile_500_250_2018_04_03.root");
  setup.add_input_file("/nfs/dust/ilc/group/ild/beyerjac/TGCAnalysis/SampleProduction/WW_charge_separated/distributions/combined/Distribution_250GeV_WW_semilep_MuAntiNu.root");
  setup.add_input_file("/nfs/dust/ilc/group/ild/beyerjac/TGCAnalysis/SampleProduction/WW_charge_separated/distributions/combined/Distribution_250GeV_WW_semilep_AntiMuNu.root");
  setup.add_energy( energy );

  spdlog::info("Selecting distributions.");
  setup.use_distr("singleWplussemileptonic");
  setup.use_distr("singleWminussemileptonic");
  // setup.use_distr("WWsemileptonic");
  setup.use_distr("WW_semilep_MuAntiNu");
  setup.use_distr("WW_semilep_AntiMuNu");
  setup.use_distr("ZZsemileptonic");
  setup.use_distr("Zhadronic");
  setup.use_distr("Zleptonic");
  
  spdlog::info("Setting up linking info.");
  
  // Set up the systematics
  setup.set_lumi(energy, 2000, 1);
  setup.add_pol("ePol-", energy, 0.8, 0.0001);
  setup.add_pol("ePol+", energy, 0.8, 0.0001);
  setup.add_pol("pPol-", energy, 0.3, 0.0001);
  setup.add_pol("pPol+", energy, 0.3, 0.0001);
  
  // Constraints on the systematics
  setup.add_lumi_constr(energy, 2000, 2000*3e-3);
  setup.add_pol_constr("ePol-", energy, 0.8, 0.8*2.5e-3);
  setup.add_pol_constr("ePol+", energy, 0.8, 0.8*2.5e-3);
  setup.add_pol_constr("pPol-", energy, 0.3, 0.3*2.5e-3);
  setup.add_pol_constr("pPol+", energy, 0.3, 0.3*2.5e-3);
  
  // Set up the desired polarisation sharings
  setup.add_pol_config("e-p+", energy, "ePol-", "pPol+", "-", "+", 0.45);
  setup.add_pol_config("e+p-", energy, "ePol+", "pPol-", "+", "-", 0.45);
  setup.add_pol_config("e-p-", energy, "ePol-", "pPol-", "-", "-", 0.05);
  setup.add_pol_config("e+p+", energy, "ePol+", "pPol+", "+", "+", 0.05);
  
  // Take cTGCs into account in fit
  setup.activate_cTGCs();
  
  // Make sure WW & ZZ have correct normalisations
  // setup.set_WW_mu_only();
  setup.set_ZZ_mu_only();
  
  // // Set chiral cross sections as free parameters (Here: use total xs and asymm instead)
  // setup.free_chiral_xsection("singleWplussemileptonic", PrEW::GlobalVar::Chiral::eLpR);
  
  // Set asymmetries and total chiral cross sections scalings as free parameters
  setup.free_asymmetry(
    "singleWminussemileptonic",
    PrEW::GlobalVar::Chiral::eLpR,
    PrEW::GlobalVar::Chiral::eRpL,
    PrEW::GlobalVar::Chiral::eLpL
  );
  setup.free_asymmetry(
    "singleWplussemileptonic",
    PrEW::GlobalVar::Chiral::eRpL,
    PrEW::GlobalVar::Chiral::eLpR,
    PrEW::GlobalVar::Chiral::eRpR
  );
  setup.free_asymmetry(
    "WW_semilep_MuAntiNu", 
    PrEW::GlobalVar::Chiral::eLpR, PrEW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "WW_semilep_AntiMuNu", 
    PrEW::GlobalVar::Chiral::eLpR, PrEW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "ZZsemileptonic", 
    PrEW::GlobalVar::Chiral::eLpR, PrEW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "Zhadronic", 
    PrEW::GlobalVar::Chiral::eLpR, PrEW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "Zleptonic", 
    PrEW::GlobalVar::Chiral::eLpR, PrEW::GlobalVar::Chiral::eRpL
  );
  setup.free_total_chiral_xsection("singleWminussemileptonic");
  setup.free_total_chiral_xsection("singleWplussemileptonic");
  setup.free_total_chiral_xsection("WW_semilep_MuAntiNu");
  setup.free_total_chiral_xsection("WW_semilep_AntiMuNu");
  setup.free_total_chiral_xsection("ZZsemileptonic");
  setup.free_total_chiral_xsection("Zhadronic");
  setup.free_total_chiral_xsection("Zleptonic");
  
  spdlog::info("Finalizing linking info.");
  setup.complete_setup(); // This must come last in linking setup

  spdlog::info("Create runner (incl. setting up toy generator).");
  PrEWUtils::Runners::ParallelRunner runner ( 
    setup, 
    minuit_minimizers, 
    prew_minimizer 
  );
  
  // spdlog::info("Set up the bin selector.");
  // PrEWUtils::DataHelp::BinSelector bin_selector(
  //   bin_cutoff_value, setup.get_pars()
  // );
  // runner.set_bin_selector( bin_selector );
  
  spdlog::info("Run toys.");
  auto results = runner.run_toy_fits( energy, n_toys, n_threads );
  
  spdlog::info("All threads done, printing first result.");
  spdlog::info(results.at(0));
  
  spdlog::info("Write results to: {}", output_path);
  PrEW::Output::Printer printer (output_path);
  printer.new_setup( energy, runner.get_data_connector() );
  printer.add_fits( results );
  printer.write();
  
  spdlog::info("Complete test done!");
}