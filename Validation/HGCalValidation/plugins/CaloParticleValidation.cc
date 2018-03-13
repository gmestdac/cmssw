// -*- C++ -*-
//
// Class:      CaloParticleValidation
// Original Author:  Marco Rovere
// Created:  Thu, 18 Jan 2018 15:54:55 GMT
//
//

#include <string>
#include <unordered_map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DQMServices/Core/interface/DQMGlobalEDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"

//
// class declaration
//

struct Histogram_CaloParticleSingle {
  ConcurrentMonitorElement eta_;
  ConcurrentMonitorElement pt_;
  ConcurrentMonitorElement energy_;
  ConcurrentMonitorElement nSimClusters_;
  ConcurrentMonitorElement nHitInSimClusters_;
  ConcurrentMonitorElement selfEnergy_; // this is the sum of the energy associated to all recHits linked to all SimClusters
  ConcurrentMonitorElement energyDifference_; // This contains (energy-selfEnergy)/energy
  ConcurrentMonitorElement eta_Zorigin_map_;
  ConcurrentMonitorElement simPFSuperClusterSize_;
  ConcurrentMonitorElement simPFSuperClusterEnergy_;
  ConcurrentMonitorElement pfcandidateType_;
  ConcurrentMonitorElement pfcandidateEnergy_;
  ConcurrentMonitorElement pfcandidatePt_;
  ConcurrentMonitorElement pfcandidateEta_;
  ConcurrentMonitorElement pfcandidatePhi_;
  ConcurrentMonitorElement pfcandidateElementsInBlocks_;
};


using Histograms_CaloParticleValidation = std::unordered_map<int, Histogram_CaloParticleSingle>;

class CaloParticleValidation : public DQMGlobalEDAnalyzer<Histograms_CaloParticleValidation> {
   public:
      explicit CaloParticleValidation(const edm::ParameterSet&);
      ~CaloParticleValidation();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void bookHistograms(DQMStore::ConcurrentBooker &,
                                  edm::Run const&,
                                  edm::EventSetup const&,
                                  Histograms_CaloParticleValidation&) const override;

      virtual void dqmAnalyze(edm::Event const&,
                              edm::EventSetup const&,
                              Histograms_CaloParticleValidation const&) const override;

      // ----------member data ---------------------------
      std::string folder_;
      std::vector<int> particles_to_monitor_;

      edm::EDGetTokenT<std::vector<SimVertex> > simVertices_;
      edm::EDGetTokenT<std::vector<CaloParticle> > caloParticles_;
      edm::EDGetTokenT<std::vector<reco::SuperCluster>> simPFClusters_;
      edm::EDGetTokenT<reco::PFCandidateCollection> simPFCandidates_;
      edm::EDGetTokenT<HGCRecHitCollection> recHitsEE_;
      edm::EDGetTokenT<HGCRecHitCollection> recHitsFH_;
      edm::EDGetTokenT<HGCRecHitCollection> recHitsBH_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
CaloParticleValidation::CaloParticleValidation(const edm::ParameterSet& iConfig)
  : folder_(iConfig.getParameter<std::string>("folder")),
  particles_to_monitor_(iConfig.getParameter<std::vector<int> >("particles_to_monitor")),
  simVertices_(consumes<std::vector<SimVertex>>(iConfig.getParameter<edm::InputTag>("simVertices"))),
  caloParticles_(consumes<std::vector<CaloParticle> >(iConfig.getParameter<edm::InputTag>("caloParticles"))),
  simPFClusters_(consumes<std::vector<reco::SuperCluster>>(iConfig.getParameter<edm::InputTag>("simPFClusters"))),
  simPFCandidates_(consumes<reco::PFCandidateCollection>(iConfig.getParameter<edm::InputTag>("simPFCandidates"))),
  recHitsEE_(consumes<HGCRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitsEE"))),
  recHitsFH_(consumes<HGCRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitsFH"))),
  recHitsBH_(consumes<HGCRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitsBH")))
{
   //now do what ever initialization is needed
}


CaloParticleValidation::~CaloParticleValidation()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// ------------ method called for each event  ------------

void
CaloParticleValidation::dqmAnalyze(edm::Event const& iEvent, edm::EventSetup const& iSetup,
                      Histograms_CaloParticleValidation const & histos) const
{
  using namespace edm;

  Handle<HGCRecHitCollection> recHitHandleEE;
  Handle<HGCRecHitCollection> recHitHandleFH;
  Handle<HGCRecHitCollection> recHitHandleBH;
  // make a map detid-rechit

  iEvent.getByToken(recHitsEE_, recHitHandleEE);
  iEvent.getByToken(recHitsFH_, recHitHandleFH);
  iEvent.getByToken(recHitsBH_, recHitHandleBH);
  const auto& rechitsEE = *recHitHandleEE;
  const auto& rechitsFH = *recHitHandleFH;
  const auto& rechitsBH = *recHitHandleBH;
  std::map<DetId, const HGCRecHit*> hitmap;
  for (unsigned int i = 0; i < rechitsEE.size(); ++i) {
    hitmap[rechitsEE[i].detid()] = &rechitsEE[i];
  }
  for (unsigned int i = 0; i < rechitsFH.size(); ++i) {
    hitmap[rechitsFH[i].detid()] = &rechitsFH[i];
  }
  for (unsigned int i = 0; i < rechitsBH.size(); ++i) {
    hitmap[rechitsBH[i].detid()] = &rechitsBH[i];
  }

  Handle<std::vector<SimVertex>> simVerticesHandle;
  iEvent.getByToken(simVertices_, simVerticesHandle);
  std::vector<SimVertex> const & simVertices = *simVerticesHandle;

  Handle<std::vector<CaloParticle> > caloParticleHandle;
  iEvent.getByToken(caloParticles_, caloParticleHandle);
  std::vector<CaloParticle> const & caloParticles = *caloParticleHandle;

  Handle<std::vector<reco::SuperCluster>> simPFClustersHandle;
  iEvent.getByToken(simPFClusters_, simPFClustersHandle);
  std::vector<reco::SuperCluster> const & simPFClusters = *simPFClustersHandle;

  Handle<reco::PFCandidateCollection> simPFCandidatesHandle;
  iEvent.getByToken(simPFCandidates_, simPFCandidatesHandle);
  reco::PFCandidateCollection const & simPFCandidates = *simPFCandidatesHandle;

  for (auto const caloParticle : caloParticles) {
    int id = caloParticle.pdgId();
    if (histos.count(id)) {
      histos.at(id).eta_.fill(caloParticle.eta());
      histos.at(id).pt_.fill(caloParticle.pt());
      histos.at(id).energy_.fill(caloParticle.energy());
      histos.at(id).nSimClusters_.fill(caloParticle.simClusters().size());
      // Find the corresponding vertex.
      histos.at(id).eta_Zorigin_map_.fill(
          simVertices.at(caloParticle.g4Tracks()[0].vertIndex()).position().z(), caloParticle.eta());
      int simHits = 0;
      float energy = 0.;
      for (auto const sc : caloParticle.simClusters()) {
        simHits += sc->hits_and_fractions().size();
        for (auto const h_and_f : sc->hits_and_fractions()) {
          if (hitmap.count(h_and_f.first))
            energy += hitmap[h_and_f.first]->energy() * h_and_f.second;
        }
      }
      histos.at(id).nHitInSimClusters_.fill((float)simHits);
      histos.at(id).selfEnergy_.fill(energy);
      histos.at(id).energyDifference_.fill(1.- energy/caloParticle.energy());
    }
  }

  // simPFSuperClusters
  for (auto const sc : simPFClusters) {
    histos.at(0).simPFSuperClusterSize_.fill((float)sc.clustersSize());
    histos.at(0).simPFSuperClusterEnergy_.fill(sc.rawEnergy());
  }

  // simPFCandidates
  int offset = 100000;
  for (auto const pfc : simPFCandidates) {
    size_t type = offset + pfc.particleId();
    histos.at(offset).pfcandidateType_.fill(type - offset);
    histos.at(type).pfcandidateEnergy_.fill(pfc.energy());
    histos.at(type).pfcandidatePt_.fill(pfc.pt());
    histos.at(type).pfcandidateEta_.fill(pfc.eta());
    histos.at(type).pfcandidatePhi_.fill(pfc.phi());
    histos.at(type).pfcandidateElementsInBlocks_.fill(pfc.elementsInBlocks().size());
  }
}


void
CaloParticleValidation::bookHistograms(DQMStore::ConcurrentBooker & ibook,
                          edm::Run const & run,
                          edm::EventSetup const & iSetup,
                          Histograms_CaloParticleValidation & histos) const
{
  for (auto const particle : particles_to_monitor_) {
    ibook.setCurrentFolder(folder_ + "CaloParticles/" + std::to_string(particle));
    histos[particle].eta_ = ibook.book1D("Eta", "Eta", 80, -4., 4.);
    histos[particle].energy_ = ibook.book1D("Energy", "Energy", 250, 0., 500.);
    histos[particle].pt_ = ibook.book1D("Pt", "Pt", 100, 0., 100.);
    histos[particle].nSimClusters_ = ibook.book1D("NSimClusters", "NSimClusters", 100, 0., 100.);
    histos[particle].nHitInSimClusters_ = ibook.book1D("NHitInSimClusters", "NHitInSimClusters", 100, 0., 100.);
    histos[particle].selfEnergy_ = ibook.book1D("SelfEnergy", "SelfEnergy", 250, 0., 500.);
    histos[particle].energyDifference_ = ibook.book1D("EnergyDifference", "(Energy-SelfEnergy)/Energy", 300, -5., 1.);
    histos[particle].eta_Zorigin_map_ = ibook.book2D("Eta vs Zorigin", "Eta vs Zorigin", 80, -4., 4., 1100, -550., 550.);
  }
  int offset = 100000;
  ibook.setCurrentFolder(folder_ + "PFCandidates");
  histos[offset].pfcandidateType_ = ibook.book1D("PFCandidateType", "PFCandidateType", 10, 0, 10);
  for (size_t type = reco::PFCandidate::h; type <= reco::PFCandidate::egamma_HF; type++) {
    ibook.setCurrentFolder(folder_ + "PFCandidates/" + std::to_string(type));
    histos[offset + type].pfcandidateEnergy_ = ibook.book1D("PFCandidateEnergy", "PFCandidateEnergy", 250, 0., 250.);
    histos[offset + type].pfcandidatePt_ = ibook.book1D("PFCandidatePt", "PFCandidatePt", 250, 0., 250.);
    histos[offset + type].pfcandidateEta_ = ibook.book1D("PFCandidateEta", "PFCandidateEta", 100, -5., 5.);
    histos[offset + type].pfcandidatePhi_ = ibook.book1D("PFCandidatePhi", "PFCandidatePhi", 100, -4., 4.);
    histos[offset + type].pfcandidateElementsInBlocks_ = ibook.book1D("PFCandidateElements", "PFCandidateElements", 20, 0., 20.);
  }
  // Folder '0' is meant to be cumulative, with no connection to pdgId
  ibook.setCurrentFolder(folder_ + std::to_string(0));
  histos[0].simPFSuperClusterSize_ = ibook.book1D("SimPFSuperClusterSize", "SimPFSuperClusterSize", 40, 0., 40.);
  histos[0].simPFSuperClusterEnergy_ = ibook.book1D("SimPFSuperClusterEnergy", "SimPFSuperClusterEnergy", 250, 0., 500.);
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
CaloParticleValidation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.add<std::string>("folder", "HGCAL/"); // Please keep the trailing '/'
  desc.add<std::vector<int> > ("particles_to_monitor", {11, -11, 13, 22, 111, 211, -211, 321, -321});
  desc.add<edm::InputTag>("simVertices", edm::InputTag("g4SimHits"));
  desc.add<edm::InputTag>("caloParticles", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<edm::InputTag>("simPFClusters", edm::InputTag("simPFProducer", "perfect"));
  desc.add<edm::InputTag>("simPFCandidates", edm::InputTag("simPFProducer"));
  desc.add<edm::InputTag>("recHitsEE", edm::InputTag("HGCalRecHit","HGCEERecHits"));
  desc.add<edm::InputTag>("recHitsFH", edm::InputTag("HGCalRecHit","HGCHEFRecHits"));
  desc.add<edm::InputTag>("recHitsBH", edm::InputTag("HGCalRecHit","HGCHEBRecHits"));
  descriptions.add("caloparticlevalidation", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(CaloParticleValidation);
