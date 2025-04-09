# Copyright (c) 2022, Kry10 Limited. All rights reserved.
#
# SPDX-License-Identifier: LicenseRef-Kry10

defmodule WxSensor do
  @behaviour KosManifest.App
  alias KosManifest.{Context, AccessControl}
  alias KosManifestSystems.Poukai

  @impl true
  @spec setup(Context.t(), Keyword.t()) :: {:ok, Context.t()} | {:error, any}
  def setup(context, opts \\ []) do

    # Add additional device resources or register clock/pinmux setups if required.
    msg_server = Keyword.get(opts, :msg_server, Poukai.msg_server())
    # rng_port = Keyword.get(opts, :rng_port, "kos_rng_protocol")
    # log_port = Keyword.get(opts, :log_port, "kos_log_protocol")
    internet_port = Keyword.get(opts, :internet_port, "kos_internet_protocol")
    i2c_port = hd(opts)

    # Set any environment variables you want your app to use for configuration here
    environ = %{
      # "KEY" => "value"
    }

    wx_sensor_binary = Path.join([Mix.Project.build_path(), "lib", "wx_sensor", "cmake", "wx_sensor"])
    wx_sensor = %{
      name: "wx_sensor",
      binary: wx_sensor_binary,
      heap_pages: 10,
      ut_large_pages: 10,
      ut_4k_pages: 10,
      max_priority: 150,
      priority: 145,
      environ: environ,
      # it seems like we need these because they are in the weather_station example
      arguments: [i2c_port],
      resources: %{ device_frames: [%{address: 0x4804C000, size: 0x1000}] },
      #
      msg_servers: [
        %{
          name: msg_server,
          dir_access_control: %{
            # rng_port => AccessControl.client(),
            # log_port => AccessControl.client(),
            internet_port => AccessControl.client(),
            i2c_port => AccessControl.client(),
          }
        }
      ]
    }

    context = context
    |> Context.put_app!(wx_sensor)

    {:ok, wx_sensor, context}
  end
end
