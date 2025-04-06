

import Config

if System.get_env("TUNNEL_PEER_ENDPOINT") do
  config :kos_manifest_systems, KosManifestSystems.Poukai,
    tunnel_settings: [
      peer_endpoint: System.get_env("TUNNEL_PEER_ENDPOINT"),
      peer_public_key: System.get_env("TUNNEL_PEER_KEY")
    ]
end

