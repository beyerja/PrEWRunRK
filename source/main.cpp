// Includes from PrEW
#include "GlobalVar/Chiral.h"
#include "Output/Printer.h"

// Includes from PrEWUtils
#include "Runners/ParallelRunner.h"
#include "Setups/RKDistrSetup.h"

#include "spdlog/spdlog.h"

int main (int /*argc*/, char **/*argv*/) {
  spdlog::set_level(spdlog::level::info);
  
  int energy = 250;
  int n_threads = 3;
  int n_toys = 10;
  std::string minimizers = "Combined";
  std::string output_path = "../output/fit_results.out";
    
  spdlog::info("Start test.");
  
  spdlog::info("Create setup.");
  PrEWUtils::Setups::RKDistrSetup setup {};
  
  spdlog::info("Add files and energies.");
  setup.add_input_file("../../PrEW/testdata/RK_examplefile_500_250_2018_04_03.root");
  setup.add_input_file("/home/jakob/Documents/DESY/MountPoints/DUSTMount/TGCAnalysis/SampleProduction/WW_charge_separated/distributions/combined/Distribution_250GeV_WW_semilep_MuAntiNu.root");
  setup.add_input_file("/home/jakob/Documents/DESY/MountPoints/DUSTMount/TGCAnalysis/SampleProduction/WW_charge_separated/distributions/combined/Distribution_250GeV_WW_semilep_AntiMuNu.root");
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
  // setup.free_chiral_xsection("singleWplussemileptonic", PREW::GlobalVar::Chiral::eLpR);
  
  // Set asymmetries and total chiral cross sections scalings as free parameters
  setup.free_asymmetry(
    "singleWminussemileptonic",
    PREW::GlobalVar::Chiral::eLpR,
    PREW::GlobalVar::Chiral::eRpL,
    PREW::GlobalVar::Chiral::eLpL
  );
  setup.free_asymmetry(
    "singleWplussemileptonic",
    PREW::GlobalVar::Chiral::eRpL,
    PREW::GlobalVar::Chiral::eLpR,
    PREW::GlobalVar::Chiral::eRpR
  );
  setup.free_asymmetry(
    "WW_semilep_MuAntiNu", 
    PREW::GlobalVar::Chiral::eLpR, PREW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "WW_semilep_AntiMuNu", 
    PREW::GlobalVar::Chiral::eLpR, PREW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "ZZsemileptonic", 
    PREW::GlobalVar::Chiral::eLpR, PREW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "Zhadronic", 
    PREW::GlobalVar::Chiral::eLpR, PREW::GlobalVar::Chiral::eRpL
  );
  setup.free_asymmetry(
    "Zleptonic", 
    PREW::GlobalVar::Chiral::eLpR, PREW::GlobalVar::Chiral::eRpL
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
  PrEWUtils::Runners::ParallelRunner runner ( setup, minimizers );

  spdlog::info("Run toys.");
  auto results = runner.run_toy_fits( energy, n_toys, n_threads );
  
  spdlog::info("All threads done, printing first result.");
  spdlog::info(results.at(0));
  
  spdlog::info("Write results to: {}", output_path);
  PREW::Output::Printer printer (output_path);
  printer.new_setup( energy, runner.get_data_connector() );
  printer.add_fits( results );
  printer.write();
  
  spdlog::info("Complete test done!");
}