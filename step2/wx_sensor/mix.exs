# Copyright (c) 2022, Kry10 Limited. All rights reserved.
#
# SPDX-License-Identifier: LicenseRef-Kry10

defmodule WxSensor.MixProject do
  use Mix.Project

  def project do
    [
      app: :wx_sensor,
      version: "0.1.0",
      compilers: [:cmake | Mix.compilers()],
      deps: deps()
    ]
  end

  def application do
    [
      extra_applications: [:logger, :kos_manifest]
    ]
  end

  defp deps do
    kos_builtins = System.get_env("KOS_BUILTINS_PATH", "KOS_BUILTINS_PATH-NOTFOUND")
    [
      {:kos_manifest_systems, path: Path.join(kos_builtins, "kos_manifest_systems")},
      {:elixir_cmake, "~> 0.8.0", runtime: false},
    ]
  end

end
