// Copyright (c) 2002-present, OpenMS Inc. -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: Aditya Sarna $
// --------------------------------------------------------------------------

#include <OpenMS/CONCEPT/ClassTest.h>
#include <OpenMS/test_config.h>

#include <OpenMS/FORMAT/ImzMLFile.h>
#include <OpenMS/KERNEL/OnDiscImzMLExperiment.h>
#include <OpenMS/KERNEL/MSExperiment.h>
#include <OpenMS/IMAGING/MSImagingExperiment.h>
#include <OpenMS/IMAGING/MSImagingGeometry.h>


using namespace OpenMS;
using namespace std;

START_TEST(ImzMLFile, "$Id$")

const String imzml_path = String(OPENMS_GET_TEST_DATA_PATH("ImzMLFile_1_Example_Continuous.imzML"));
const String imzml_processed_path = String(OPENMS_GET_TEST_DATA_PATH("ImzMLFile_2_Example_Processed.imzML"));


START_SECTION(void load(const String& filename, MSExperiment& exp))
{
  MSExperiment exp;
  ImzMLFile f;
  f.setLogType(ProgressLogger::CMD);
  f.load(imzml_path, exp);

  TEST_EQUAL(exp.getNrSpectra() > 0, true)
  if (!exp.empty())
  {
    TEST_EQUAL(exp[0].size() > 0, true)

    if (exp[0].metaValueExists("imzml:x"))
    {
      TEST_EQUAL(exp[0].getMetaValue("imzml:x"), 1)
      TEST_EQUAL(exp[0].getMetaValue("imzml:y"), 1)
    }
  }
}
END_SECTION


START_SECTION(OnDiscImzMLExperiment random access)
{
  OnDiscImzMLExperiment od;
  od.open(imzml_path);

  TEST_EQUAL(od.getNrSpectra() > 0, true)

  if (od.getNrSpectra() > 0)
  {
    MSSpectrum s = od.getSpectrum(0);
    TEST_EQUAL(s.size() > 0, true)

    const auto& e0 = od.getIndex(0);
    MSSpectrum sc = od.getSpectrumAtCoord(e0.x, e0.y, e0.z);
    TEST_EQUAL(sc.size() > 0, true)
  }
}
END_SECTION


START_SECTION(void load(const String& filename, Interfaces::IMSDataConsumer& consumer))
{
  struct CountConsumer : public Interfaces::IMSDataConsumer
  {
    Size count {0};
    void setExpectedSize(Size, Size) override {}
    void setExperimentalSettings(const ExperimentalSettings&) override {}
    void consumeChromatogram(MSChromatogram&) override {}
    void consumeSpectrum(MSSpectrum&) override { ++count; }
  } consumer;

  ImzMLFile f;
  f.load(imzml_path, consumer);
  TEST_EQUAL(consumer.count > 0, true)
}
END_SECTION


START_SECTION(void load Example_Processed imzML)
{
  MSExperiment exp;
  ImzMLFile f;
  f.load(imzml_processed_path, exp);
  TEST_EQUAL(exp.getNrSpectra() > 0, true)
  if (!exp.empty())
  {
    TEST_EQUAL(exp[0].size() > 0, true)
  }
}
END_SECTION


START_SECTION(void load(const String& filename, MSImagingExperiment& exp))
{
  MSImagingExperiment imaging;
  ImzMLFile f;
  f.load(imzml_path, imaging);

  TEST_EQUAL(imaging.getNumberOfSpectra() > 0, true)
  TEST_EQUAL(imaging.getGeometry().getWidth(), 3)
  TEST_EQUAL(imaging.getGeometry().getHeight(), 3)
  TEST_EQUAL(imaging.hasPixel(0, 0), true)
  if (imaging.hasPixel(0, 0))
  {
    TEST_EQUAL(imaging.getSpectrum(0, 0).size() > 0, true)
  }
  TEST_EQUAL(imaging.hasPixel(2, 2), true)
  if (imaging.hasPixel(2, 2))
  {
    TEST_EQUAL(imaging.getSpectrum(2, 2).size() > 0, true)
  }
}
END_SECTION


START_SECTION(void buildImagingGeometry(const MSExperiment& exp, MSImagingGeometry& geom))
{
  MSExperiment exp;
  ImzMLFile f;
  f.load(imzml_path, exp);

  MSImagingGeometry geom;
  ImzMLFile::buildImagingGeometry(exp, geom);
  TEST_EQUAL(geom.getNumberOfPixels(), exp.getNrSpectra())
  if (geom.getNumberOfPixels() > 0)
  {
    TEST_EQUAL(geom.getSpectrumIndex(0, 0), 0)
  }
}
END_SECTION


START_SECTION(void store round-trip continuous imzML)
{
  MSExperiment original;
  ImzMLFile f;
  f.load(imzml_path, original);

  String tmp_imzml;
  NEW_TMP_FILE_EXT(tmp_imzml, ".imzML");
  f.store(tmp_imzml, original);

  MSExperiment reloaded;
  f.load(tmp_imzml, reloaded);

  TEST_EQUAL(reloaded.getNrSpectra(), original.getNrSpectra())
  if (!original.empty() && !reloaded.empty())
  {
    TEST_EQUAL(reloaded[0].size(), original[0].size())
    TEST_REAL_SIMILAR(reloaded[0][0].getMZ(), original[0][0].getMZ())
    TEST_REAL_SIMILAR(reloaded[0][0].getIntensity(), original[0][0].getIntensity())
  }
  TEST_EQUAL(reloaded.metaValueExists("imzml:imaging_mode"), true)
  TEST_EQUAL(reloaded.getMetaValue("imzml:imaging_mode"), "continuous")
}
END_SECTION


START_SECTION(void store round-trip processed imzML)
{
  MSExperiment original;
  ImzMLFile f;
  f.load(imzml_processed_path, original);

  String tmp_imzml;
  NEW_TMP_FILE_EXT(tmp_imzml, ".imzML");
  f.store(tmp_imzml, original);

  MSExperiment reloaded;
  f.load(tmp_imzml, reloaded);

  TEST_EQUAL(reloaded.getNrSpectra(), original.getNrSpectra())
  if (!original.empty() && !reloaded.empty())
  {
    TEST_EQUAL(reloaded[0].size(), original[0].size())
    TEST_REAL_SIMILAR(reloaded[0][0].getMZ(), original[0][0].getMZ())
  }
  TEST_EQUAL(reloaded.getMetaValue("imzml:imaging_mode"), "processed")
}
END_SECTION

END_TEST
