# Validation and boundaries

Application/SDK **0.1.1**, Microsoft-signed driver **0.1.0.0**, Windows x64. The complete release includes actual self-contained UI/console installers; the kernel implementation stays private.

## Signed package identity

The returned INF is byte-identical to the submission. Windows catalog hashing confirms that protected SYS content matches the qualified original image, apart from Authenticode exclusions. Microsoft kernel-policy catalog trust, hardware-publisher identity and exact INF/SYS membership were verified on the development host and again inside the guest. Both installers were inspected as PE resources without executing host installer code; each embeds the exact three returned files.

- Original qualified SYS SHA-256: `0A941B12D3B8363A559362F06EB103C348D3BCBE921CBA9F385E10742162DA31`.
- Returned signed SYS SHA-256: `335258d20a93d1c2b11a390c2b15bd7868e9ac20e77bbb0fb5dd78855d3ea325`.
- No kernel recompilation, catalog regeneration, lab certificate or replacement driver image was used for the return.

## Actual signed Windows tests

The exact returned image and resource-verified console installer passed isolated **Windows 10 build 19045 x64** campaigns on 2026-10-25: normal: **3242 checks**, targeted Driver Verifier: **3239 checks**. Test-signing was explicitly disabled before installation in both campaigns; the guest's pre-existing Secure Boot setting was not changed.

Each campaign exercised both-direction Ethernet capture, 256 independently observed raw sends, 1,024 receive indications, genuine Windows UDP, bounded ring overflow, malformed requests, forged/replayed permissions, expiry/renewal, owner cleanup and an owner-open PnP restart of the exact synthetic adapter. Capture used the independent Titan Virtual Network fixture, not a physical network adapter.

Restricted-token control access, administrator zero/read-only/write-only opens and a second control owner were refused. Actual console setup installation and uninstallation were exercised. Every owned package was removed, its service unloaded and temporary network configuration cleaned up. No new minidump was observed. The guest returned to its original powered-off baseline, with all nine historical checkpoints retained. No driver, certificate, host boot setting or host network policy was installed or changed on the development PC.

Standard Driver Verifier targeted the two candidate drivers and their exact synthetic fixture. Special pool, forced IRQL, pool tracking, I/O, deadlock, DMA, security, miscellaneous and DDI checks were enabled. NDIS/WIFI-specific checking, low-resource simulation and prolonged pressure were not enabled. The UI and console setups share one installation core; UI click automation is not claimed.

## Offline checks

40,009 parser/round-trip/mutation checks and eight installer identity/refusal checks; two CTest cases pass in the signed user-mode build. The unchanged C++ code also passed its separate AddressSanitizer configuration. Native builds use W4/WX, CFG, ASLR, NX and static CRT. Kernel PREfast reports zero defects; the original InfVerif/catalog and submission evidence are preserved. These source builds and protocol tests do not install a host driver or alter network policy.

## Scope

Bounded Ethernet capture and raw-frame transmission on one explicitly selected adapter. No Npcap/libpcap drop-in compatibility, Wi-Fi monitor mode, line-rate or physical-adapter qualification claim.

Windows 11, Secure Boot/HVCI, representative physical NICs, offloads, concurrent security/filter products, suspend/resume, recovery and prolonged workloads remain separate qualification work. Functional delay checks are not latency/throughput benchmarks. Microsoft signing is not WHQL certification, exhaustive testing, zero overhead or a guarantee of universal compatibility.

Signed per-handle permissions last at most one hour and can be explicitly renewed while a product key is enabled. Revocation refuses new grants rather than instantly cancelling accepted permissions. Private kernel source, CABs, PDBs, lab material and signing credentials are excluded from public repositories and downloads.
