# Copyright (c) 2022, Kry10 Limited. All rights reserved.
#
# SPDX-License-Identifier: LicenseRef-Kry10

defmodule WxStationManifest do
  alias KosManifest.Context
  alias KosManifestSystems.Poukai
  alias Kos.Tunnel
  import Context

  @behaviour KosManifest.System

  @impl true
  def kos_base_system(), do: Poukai

  @impl true
  def setup(context) do

    i2c_bus = Application.get_env(:manifest, __MODULE__)
      |> Keyword.get(:i2c_bus)
    i2c_port = "kos_#{i2c_bus}_protocol"
 
    # Remove this line when you want to enforce signed upgrades
    # See `mix help kos.manifest.sign` for commands for signing a manifest.
    {:ok, context} = set_kos_options(context, %{disable_signing: true})
    
    # Add new applications here:
    # {:ok, context, _} = put_app(context, MyApp)
    # {:ok, context, _} = put_app(context, WxSensor)

    context = context
    |> put_port!(i2c_port, Poukai.msg_server())
    |> put_app!(WxSensor, [i2c_port])
    
    # Checks to make sure that each app has valid KOS tunnel IP configurations
    {:ok, context} = Tunnel.check_tunnel_ips(context)

    {:ok, context}
  end
end
