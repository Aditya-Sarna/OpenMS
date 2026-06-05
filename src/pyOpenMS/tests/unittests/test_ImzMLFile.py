import os
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from test_data_paths import get_class_test_data_dir

import pyopenms


class TestImzMLFile(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        try:
            data_dir = get_class_test_data_dir()
        except RuntimeError as e:
            raise unittest.SkipTest(str(e)) from e
        cls.imzml_path = os.path.join(
            data_dir,
            "ImzMLFile_1_Example_Continuous.imzML",
        )
        if not os.path.isfile(cls.imzml_path):
            raise unittest.SkipTest(f"imzML test data not found: {cls.imzml_path}")

    def test_imzml_file_load(self):
        exp = pyopenms.MSExperiment()
        pyopenms.ImzMLFile().load(self.imzml_path, exp)
        self.assertGreater(exp.getNrSpectra(), 0)
        self.assertGreater(exp.getSpectrum(0).size(), 0)

    def test_file_handler_load(self):
        exp = pyopenms.MSExperiment()
        pyopenms.FileHandler().loadExperiment(self.imzml_path, exp)
        self.assertGreater(exp.getNrSpectra(), 0)

    def test_file_types(self):
        t = pyopenms.FileHandler.getType(self.imzml_path)
        self.assertEqual(t, pyopenms.FileType.IMZML)

    def test_on_disc(self):
        od = pyopenms.OnDiscImzMLExperiment()
        try:
            od.open(self.imzml_path)
            self.assertTrue(od.isOpen())
            self.assertGreater(od.getNrSpectra(), 0)
            s = od.getSpectrum(0)
            self.assertGreater(s.size(), 0)
        finally:
            od.close()
            self.assertFalse(od.isOpen())

    def test_imaging_experiment_ram_lookup(self):
        imaging = pyopenms.MSImagingExperiment()
        pyopenms.ImzMLFile().load(self.imzml_path, imaging)
        self.assertEqual(imaging.getGeometry().getWidth(), 3)
        self.assertEqual(imaging.getGeometry().getHeight(), 3)
        self.assertTrue(imaging.hasPixel(0, 0))
        self.assertGreater(imaging.getSpectrum(0, 0).size(), 0)
        self.assertTrue(imaging.hasPixel(2, 2))
        self.assertGreater(imaging.getSpectrum(2, 2).size(), 0)

    def test_load_spectra_index(self):
        meta, index = pyopenms.ImzMLFile().loadSpectraIndex(self.imzml_path)
        self.assertEqual(meta.imaging_mode, "continuous")
        self.assertEqual(meta.max_count_x, 3)
        self.assertEqual(meta.max_count_y, 3)
        self.assertEqual(len(index), 9)
        self.assertEqual(index[0].x, 1)
        self.assertEqual(index[0].y, 1)
        self.assertGreater(index[0].mz_length, 0)

    def test_on_disc_index_and_meta(self):
        od = pyopenms.OnDiscImzMLExperiment()
        try:
            od.open(self.imzml_path)
            meta = od.getImzMLMeta()
            self.assertEqual(meta.imaging_mode, "continuous")
            entry = od.getIndex(0)
            self.assertEqual(entry.x, 1)
            self.assertEqual(entry.y, 1)
            self.assertGreater(entry.mz_length, 0)
        finally:
            od.close()

    def test_store_round_trip(self):
        import tempfile

        exp = pyopenms.MSExperiment()
        pyopenms.ImzMLFile().load(self.imzml_path, exp)
        with tempfile.NamedTemporaryFile(suffix=".imzML", delete=False) as tmp:
            out_path = tmp.name
        try:
            pyopenms.ImzMLFile().store(out_path, exp)
            reloaded = pyopenms.MSExperiment()
            pyopenms.ImzMLFile().load(out_path, reloaded)
            self.assertEqual(reloaded.getNrSpectra(), exp.getNrSpectra())
            self.assertGreater(reloaded.getSpectrum(0).size(), 0)
            self.assertEqual(str(reloaded.getMetaValue("imzml:imaging_mode")), "continuous")
        finally:
            os.remove(out_path)
            ibd_path = out_path[:-6] + ".ibd" if out_path.lower().endswith(".imzml") else out_path + ".ibd"
            if os.path.isfile(ibd_path):
                os.remove(ibd_path)


if __name__ == "__main__":
    unittest.main()
