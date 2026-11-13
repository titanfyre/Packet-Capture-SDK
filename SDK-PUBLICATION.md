# SDK publication status

Current source layout and registry coverage. A GitHub download is distinct from publication on a central registry.

| Target | Status | Package / documentation |
| --- | --- | --- |
| CPP/vcpkg | published_custom_registry | packet-capture-sdk |
| CPP/conan | public_recipe_builds_locally |  [Source](packaging/native/) |
| CPP/nuget | paused_pending_owner_email_confirmation | Packet-Capture-SDK.Native |
| NET/nuget | built_verified_publication_on_hold_by_owner | PacketCaptureSDK.SDK [Source](Packet-Capture-SDK-NET-nuget/) |
| Python/pypi | built_fixture_and_production_verified | packet-capture-sdk [Source](Packet-Capture-SDK-Python-pypi/) |
| Python/conda-forge | built_tested_requires_source_publication_and_official_review | packet-capture-sdk [Source](packaging/conda/) |
| Rust/cargo | published_install_verified | [packet-capture-sdk](https://crates.io/crates/packet-capture-sdk) [Source](Packet-Capture-SDK-Rust-cargo/) |
| JavaScript/npm | awaiting_browser_publication_authentication | [packet-capture-sdk](https://www.npmjs.com/package/packet-capture-sdk) [Source](Packet-Capture-SDK-JavaScript-npm/) |
| Java/maven | published_install_verified | [packet-capture-sdk](https://central.sonatype.com/artifact/io.github.titancsa/packet-capture-sdk/1.0.0) [Source](Packet-Capture-SDK-Java-maven/) |
| Java/jitpack | published_install_verified | [packet-capture-sdk](https://jitpack.io/#titanfyre/Packet-Capture-SDK/java-sdk-v1.0.0) [Source](Packet-Capture-SDK-Java-maven/) |
| Go/modules | published_install_verified | [packet-capture-sdk](https://pkg.go.dev/github.com/titanfyre/Packet-Capture-SDK/Packet-Capture-SDK-Go-modules@v1.0.0) [Source](Packet-Capture-SDK-Go-modules/) |
| Go/pkg-go-dev | indexed_verified | [Documentation](https://pkg.go.dev/github.com/titanfyre/Packet-Capture-SDK/Packet-Capture-SDK-Go-modules@v1.0.0) |
| PowerShell/gallery | published_install_verified | [PacketCaptureSDK](https://www.powershellgallery.com/packages/PacketCaptureSDK/1.0.0) [Source](Packet-Capture-SDK-PowerShell-gallery/) |
| Dart/pub-dev | built_fixture_and_production_verified | packet_capture_sdk [Source](Packet-Capture-SDK-Dart-pub-dev/) |
| Ruby/rubygems | published_install_verified | [packet-capture-sdk](https://rubygems.org/gems/packet-capture-sdk) [Source](Packet-Capture-SDK-Ruby-rubygems/) |
| PHP/packagist | composer_source_and_tag_published | [titancsa/packet-capture-sdk](https://packagist.org/packages/titancsa/packet-capture-sdk) |

Native SDK source and CMake/Conan/vcpkg/NuGet recipes share [`packaging/native`](packaging/native/LAYOUT.md). Previous release archives and tags remain unchanged.

Application and driver prerequisites and original licensing remain those documented by each SDK. No driver installer is bundled with these source packages.
