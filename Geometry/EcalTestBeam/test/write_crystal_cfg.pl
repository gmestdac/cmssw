#! /usr/bin/perl -w

# build the cff file for all the crystals corresponding to
# their nominal maximum reference point as stored in the table                                                                                
# F. Cossutti - 23-Aug-2006 15:19


use diagnostics;
use strict;
                                                                                
my $file = "BarrelSM1CrystalCenterElectron120GeV.dat";

if (! open(INPUT,"<$file") ) {
    print STDERR "Can't open input file $file: $!\n";
    exit 1;
   }
                                                                                 
    while(<INPUT>){
        chomp(my $line = $_);

        print $line, "\n";

        my $crystal = my $eta = my $phi = ();

        ($crystal,$eta,$phi) = split(' ', $line, 3); 

        my $filename = "crystal".$crystal."_cff.py";

        open(OUTFILE, ">$filename");
        print OUTFILE "import FWCore.ParameterSet.Config as cms\n";
        print OUTFILE "                                        \n";   
        print OUTFILE "common_beam_direction_parameters = cms.PSet(\n";
        print OUTFILE "    MinEta = cms.untracked.double(".$eta,"),\n";
        print OUTFILE "    MaxEta = cms.untracked.double(".$eta,"),\n";
        print OUTFILE "    MinPhi = cms.untracked.double(".$phi,"),\n";
        print OUTFILE "    MaxPhi = cms.untracked.double(".$phi,"),\n";
        print OUTFILE "    BeamMeanX = cms.untracked.double(0.0),\n";
        print OUTFILE "    BeamMeanY = cms.untracked.double(0.0),\n";
        print OUTFILE "    BeamPosition = cms.untracked.double(-26733.5)\n";
        print OUTFILE ")\n";
 
    }
    close(INPUT);
 
exit 0;
