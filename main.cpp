#include "tinytiffwriter.h"

#include "TH2D.h"
#include "TMath.h"
#include "TFile.h"
#include "TImage.h"

#include <iostream>

void ApplySaturation(TH2D* hist, double sat_value) {
    for (unsigned int i = 0; i < hist->GetSize(); ++i) {
        if (hist->GetBinContent(i) > sat_value) hist->SetBinContent(i, sat_value);
    }
}

TH2I* ApplyPixelValueConversion(TH2D* hist, uint32_t resolutionDegree, double sat_value) {
    uint16_t nbins = TMath::Power(2, (int) resolutionDegree) - 1;
    double bin_width = sat_value / (double) nbins;
    TH2I* h_pixel = (TH2I*) hist->Clone();
    for (unsigned int i = 0; i < hist->GetSize(); ++i) {
        double raw_value = hist->GetBinContent(i);
        uint16_t pixel_value = (uint16_t) (raw_value / bin_width);
        h_pixel->SetBinContent(i,  pixel_value);
    }

    return h_pixel;
}

void Convert(TH2I* hist, uint32_t bitsPerSample) {
    uint32_t nx = hist->GetNbinsX() + 2;
    uint32_t ny = hist->GetNbinsY() + 2;
    uint16_t nSamples = 1;
    TinyTIFFWriterFile* tif = TinyTIFFWriter_open(Form("%s.tif", hist->GetName()), bitsPerSample,
                                                  TinyTIFFWriter_UInt, nSamples, nx, ny,
                                                  TinyTIFFWriter_Greyscale);
    if (tif) {
        uint16_t* data = (uint16_t*) malloc(nx*ny*sizeof(uint16_t));
        for (long int i = 0; i < nx * ny; i++) {
            data[i] = (uint16_t) hist->GetBinContent(i);
        }
        TinyTIFFWriter_writeImage(tif, data);
        TinyTIFFWriter_close(tif);
        free(data);
    }
}

int main() {
    TFile *inroot = new TFile("ImageFormat.root", "READ");

    TH2D* h_DRZCube    = (TH2D*) inroot->Get("DRZCube");
    TH2D* h_DRZPlate1  = (TH2D*) inroot->Get("DRZPlate1");
    TH2D* h_DRZPlate2  = (TH2D*) inroot->Get("DRZPlate2");
    TH2D* h_ImagePlate = (TH2D*) inroot->Get("ImagePlate");

    double drzcube_sat = 2.2E9;
    double drzplate1_sat = 150E6;
    double drzplate2_sat = 90E6;
    double imageplate_sat = 55E6; // no saturation on IP

    uint16_t drzcube_bitsPerSample = 16;
    uint16_t drzplate1_bitsPerSample = 16;
    uint16_t imageplate_bitsPerSample = 16;
    uint16_t drzplate2_bitsPerSample = 16;

    ApplySaturation(h_DRZCube, drzcube_sat);
    ApplySaturation(h_DRZPlate1, drzplate1_sat);
    ApplySaturation(h_ImagePlate, imageplate_sat);
    ApplySaturation(h_DRZPlate2, drzplate2_sat);

    TH2I* h_DRZCube_pixel    = ApplyPixelValueConversion(h_DRZCube, drzcube_bitsPerSample, drzcube_sat);
    TH2I* h_DRZPlate1_pixel  = ApplyPixelValueConversion(h_DRZPlate1, drzplate1_bitsPerSample, drzplate1_sat);
    TH2I* h_ImagePlate_pixel = ApplyPixelValueConversion(h_ImagePlate, imageplate_bitsPerSample, imageplate_sat);
    TH2I* h_DRZPlate2_pixel  = ApplyPixelValueConversion(h_DRZPlate2, drzplate2_bitsPerSample, drzplate2_sat);

    Convert(h_DRZCube_pixel, drzcube_bitsPerSample);
    Convert(h_DRZPlate1_pixel, drzplate1_bitsPerSample);
    Convert(h_ImagePlate_pixel, imageplate_bitsPerSample);
    Convert(h_DRZPlate2_pixel, drzplate2_bitsPerSample);

    inroot->Close();

    return 0;
}
