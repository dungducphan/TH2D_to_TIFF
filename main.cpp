#include "tinytiffwriter.h"

#include "TH2D.h"
#include "TMath.h"
#include "TFile.h"
#include "TImage.h"

void ApplySaturation(TH2D* hist, double sat_value) {
    for (unsigned int i = 0; i < hist->GetSize(); ++i) {
        if (hist->GetBinContent(i) > sat_value) hist->SetBinContent(i, sat_value);
    }
}

TH2D* ApplyPixelValueConversion(TH2D* hist, int resolutionDegree, double sat_value) {
    int nbins = TMath::Power(2, resolutionDegree) - 1;
    double bin_width = sat_value / (double) nbins;
    TH2D* h_pixel = (TH2D*) hist->Clone();
    for (unsigned int i = 0; i < hist->GetSize(); ++i) {
        double raw_value = hist->GetBinContent(i);
        int pixel_value = (int) raw_value / bin_width;
        h_pixel->SetBinContent(i, (double) pixel_value);
    }

    return h_pixel;
}

int main() {
    TFile *inroot = new TFile("ImageFormat.root", "READ");

    TH2D* h_DRZCube    = (TH2D*) inroot->Get("DRZCube");
    TH2D* h_DRZPlate1  = (TH2D*) inroot->Get("DRZPlate1");
    TH2D* h_DRZPlate2  = (TH2D*) inroot->Get("DRZPlate2");
    TH2D* h_ImagePlate = (TH2D*) inroot->Get("ImagePlate");

    double drzcube_sat = 210E6;
    double drzplate1_sat = 18E6;
    double drzplate2_sat = 8E6;
    double imageplate_sat = 12E6; // no saturation on IP

    ApplySaturation(h_DRZCube, drzcube_sat);
    ApplySaturation(h_DRZPlate1, drzplate1_sat);
    ApplySaturation(h_ImagePlate, imageplate_sat);
    ApplySaturation(h_DRZPlate2, drzplate2_sat);

    TH2D* h_DRZCube_pixel    = ApplyPixelValueConversion(h_DRZCube, 18, drzcube_sat);
    TH2D* h_DRZPlate1_pixel  = ApplyPixelValueConversion(h_DRZPlate1, 20, drzplate1_sat);
    TH2D* h_ImagePlate_pixel = ApplyPixelValueConversion(h_ImagePlate, 16, imageplate_sat);
    TH2D* h_DRZPlate2_pixel  = ApplyPixelValueConversion(h_DRZPlate2, 20, drzplate2_sat);

    /*
    TImage* img_drzcube = TImage::Create();
    img_drzcube->SetImage((const Double_t*)h_DRZCube_pixel->GetArray(),
                          h_DRZCube_pixel->GetNbinsX() + 2,
                          h_DRZCube_pixel->GetNbinsY() + 2,
                          0);
    // img_drzcube->Gray(kTRUE);
    img_drzcube->WriteImage("DRZ_Cube.png", TImage::kPng);
    */

    uint32_t nx = h_DRZCube_pixel->GetNbinsX() + 2;
    uint32_t ny = h_DRZCube_pixel->GetNbinsY() + 2;
    uint16_t bitsPerSample = 18;
    uint16_t nSamples = 1;
    TinyTIFFWriterFile* tif = TinyTIFFWriter_open("DRZCube.tif", bitsPerSample, TinyTIFFWriter_UInt, nSamples, nx, ny, TinyTIFFWriter_Greyscale);
    if (tif) {
        uint32_t* data = (uint32_t*)calloc(nx*ny,sizeof(uint32_t));
        for (unsigned int i = 0; i < h_DRZCube_pixel->GetSize(); ++i) {
            data[i] = (uint32_t ) h_DRZCube_pixel->GetBinContent(i);
        }
        TinyTIFFWriter_writeImage(tif, data);
        free(data);
        TinyTIFFWriter_close(tif);
    }

    inroot->Close();

    return 0;
}
