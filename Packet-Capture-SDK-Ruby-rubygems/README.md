# Packet-Capture-SDK Ruby SDK

`gem install packet-capture-sdk`

Windows x64 Ruby3.1+.

```ruby
require "packet_capture_sdk"
PacketCaptureSDKSDK::Client.open { |client| p client.call("status") }
```

[API and operation schema](docs/API.md). Matching driver installed separately.
