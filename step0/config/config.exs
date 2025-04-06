
import Config

# WARNING: this MUST only be set to true for development devices
# Ensure it is removed or set to false before building the release for
# production devices.
config :kos_manifest, insecure_remote_iex_enabled: false


config :kos_manifest, default_module: WxStationManifest

config :kos_manifest_systems, KosManifestSystems.Poukai, 
  tunnel_ips:  %{
          "Poukai.admin" => %{ADDR: "192.168.2.6", MASK: "255.255.255.0", GW: "192.168.2.100"},
        },
  tunnel_settings: []
