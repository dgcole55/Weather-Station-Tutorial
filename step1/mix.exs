# Copyright (c) 2022, Kry10 Limited. All rights reserved.
#
# SPDX-License-Identifier: LicenseRef-Kry10

defmodule WxStationManifest.MixProject do
  use Mix.Project

  def project do
    [
      app: :manifest,
      version: "0.1.0",
      deps: deps(),
      aliases: aliases()
    ]
  end

  defp aliases do
    [
      "kos.build": ["deps.get", "kos.manifest"]
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger, :kos_manifest],
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    kos_builtins = System.get_env("KOS_BUILTINS_PATH", "KOS_BUILTINS_PATH-NOTFOUND")
    [
      # {:kos_manifest, path: Path.join(kos_builtins, "kos_manifest")},
      {:kos_manifest_systems, path: Path.join(kos_builtins, "kos_manifest_systems")},
      {:kos_i2c, path: Path.join(kos_builtins, "kos_i2c")},
    ]
  end
end
