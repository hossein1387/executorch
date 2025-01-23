import importlib.util
import os
import subprocess
import sys
import unittest
from unittest.mock import mock_open, patch

import pytest

# Dynamically import the script
script_path = os.path.join(".ci", "scripts", "gather_benchmark_configs.py")
spec = importlib.util.spec_from_file_location("gather_benchmark_configs", script_path)
gather_benchmark_configs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gather_benchmark_configs)


@pytest.mark.skipif(
    sys.platform != "linux", reason="The script under test runs on Linux runners only"
)
class TestGatehrBenchmarkConfigs(unittest.TestCase):

    def test_extract_all_configs_android(self):
        android_configs = gather_benchmark_configs.extract_all_configs(
            gather_benchmark_configs.BENCHMARK_CONFIGS, "android"
        )
        self.assertIn("xnnpack_q8", android_configs)
        self.assertIn("qnn_q8", android_configs)
        self.assertIn("llama3_spinquant", android_configs)
        self.assertIn("llama3_qlora", android_configs)

    def test_extract_all_configs_ios(self):
        ios_configs = gather_benchmark_configs.extract_all_configs(
            gather_benchmark_configs.BENCHMARK_CONFIGS, "ios"
        )

        self.assertIn("xnnpack_q8", ios_configs)
        self.assertIn("coreml_fp16", ios_configs)
        self.assertIn("mps", ios_configs)
        self.assertIn("llama3_coreml_ane", ios_configs)
        self.assertIn("llama3_spinquant", ios_configs)
        self.assertIn("llama3_qlora", ios_configs)

    def test_generate_compatible_configs_llama_model(self):
        model_name = "meta-llama/Llama-3.2-1B"
        target_os = "ios"
        result = gather_benchmark_configs.generate_compatible_configs(
            model_name, target_os
        )
        expected = ["llama3_fb16", "llama3_coreml_ane"]
        self.assertEqual(result, expected)

        target_os = "android"
        result = gather_benchmark_configs.generate_compatible_configs(
            model_name, target_os
        )
        expected = ["llama3_fb16"]
        self.assertEqual(result, expected)

    def test_generate_compatible_configs_quantized_llama_model(self):
        model_name = "meta-llama/Llama-3.2-1B-Instruct-SpinQuant_INT4_EO8"
        result = gather_benchmark_configs.generate_compatible_configs(model_name, None)
        expected = ["llama3_spinquant"]
        self.assertEqual(result, expected)

        model_name = "meta-llama/Llama-3.2-1B-Instruct-QLORA_INT4_EO8"
        result = gather_benchmark_configs.generate_compatible_configs(model_name, None)
        expected = ["llama3_qlora"]
        self.assertEqual(result, expected)

    def test_generate_compatible_configs_non_genai_model(self):
        model_name = "mv2"
        target_os = "xplat"
        result = gather_benchmark_configs.generate_compatible_configs(
            model_name, target_os
        )
        expected = ["xnnpack_q8"]
        self.assertEqual(result, expected)

        target_os = "android"
        result = gather_benchmark_configs.generate_compatible_configs(
            model_name, target_os
        )
        expected = ["xnnpack_q8", "qnn_q8"]
        self.assertEqual(result, expected)

        target_os = "ios"
        result = gather_benchmark_configs.generate_compatible_configs(
            model_name, target_os
        )
        expected = ["xnnpack_q8", "coreml_fp16", "mps"]
        self.assertEqual(result, expected)

    def test_generate_compatible_configs_unknown_model(self):
        model_name = "unknown_model"
        target_os = "ios"
        result = gather_benchmark_configs.generate_compatible_configs(
            model_name, target_os
        )
        self.assertEqual(result, [])

    def test_is_valid_huggingface_model_id_valid(self):
        valid_model = "meta-llama/Llama-3.2-1B"
        self.assertTrue(
            gather_benchmark_configs.is_valid_huggingface_model_id(valid_model)
        )

    @patch("builtins.open", new_callable=mock_open)
    @patch("os.getenv", return_value=None)
    def test_set_output_no_github_env(self, mock_getenv, mock_file):
        with patch("builtins.print") as mock_print:
            gather_benchmark_configs.set_output("test_name", "test_value")
            mock_print.assert_called_with("::set-output name=test_name::test_value")

    def test_device_pools_contains_all_devices(self):
        expected_devices = [
            "apple_iphone_15",
            "apple_iphone_15+ios_18",
            "samsung_galaxy_s22",
            "samsung_galaxy_s24",
            "google_pixel_8_pro",
        ]
        for device in expected_devices:
            self.assertIn(device, gather_benchmark_configs.DEVICE_POOLS)

    def test_gather_benchmark_configs_cli(self):
        args = {
            "models": "mv2,dl3",
            "os": "ios",
            "devices": "apple_iphone_15",
            "configs": None,
        }

        cmd = ["python", ".ci/scripts/gather_benchmark_configs.py"]
        for key, value in args.items():
            if value is not None:
                cmd.append(f"--{key}")
                cmd.append(value)

        result = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, f"Error: {result.stderr}")
        self.assertIn('"model": "mv2"', result.stdout)
        self.assertIn('"model": "dl3"', result.stdout)
        self.assertIn('"config": "coreml_fp16"', result.stdout)
        self.assertIn('"config": "xnnpack_q8"', result.stdout)
        self.assertIn('"config": "mps"', result.stdout)

    def test_gather_benchmark_configs_cli_specified_configs(self):
        args = {
            "models": "mv2,dl3",
            "os": "ios",
            "devices": "apple_iphone_15",
            "configs": "coreml_fp16,xnnpack_q8",
        }

        cmd = ["python", ".ci/scripts/gather_benchmark_configs.py"]
        for key, value in args.items():
            if value is not None:
                cmd.append(f"--{key}")
                cmd.append(value)

        result = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, f"Error: {result.stderr}")
        self.assertIn('"model": "mv2"', result.stdout)
        self.assertIn('"model": "dl3"', result.stdout)
        self.assertIn('"config": "coreml_fp16"', result.stdout)
        self.assertIn('"config": "xnnpack_q8"', result.stdout)
        self.assertNotIn('"config": "mps"', result.stdout)

    def test_gather_benchmark_configs_cli_specified_configs_raise(self):
        args = {
            "models": "mv2,dl3",
            "os": "ios",
            "devices": "apple_iphone_15",
            "configs": "qnn_q8",
        }

        cmd = ["python", ".ci/scripts/gather_benchmark_configs.py"]
        for key, value in args.items():
            if value is not None:
                cmd.append(f"--{key}")
                cmd.append(value)

        result = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, f"Error: {result.stderr}")
        self.assertIn("Unsupported config 'qnn_q8'", result.stderr)


if __name__ == "__main__":
    unittest.main()
