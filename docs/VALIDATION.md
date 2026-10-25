# Validation and limits

The private driver targets Windows 10 build 19041+ x64 and later Windows releases; target support is not a qualification claim. Microsoft signing is pending. The public preview excludes the driver and setup executables.

## Offline checks

- Native user-mode targets build with MSVC W4/WX, CFG, ASLR, NX and static CRT.
- Two CTest cases pass normally and in a separate AddressSanitizer configuration: 40,009 deterministic parser boundary/round-trip/mutation checks and eight read-only INF identity/unsigned-signature refusal checks. They do not load a driver or alter network configuration.
- The NDIS kernel builds with W4/WX, CFG and PREfast, with zero reported analysis defects.
- InfVerif `/u` passes. InfVerif `/w /wbuild 10.0.25319` checks the modern isolated model while retaining the documented Microsoft LWF downlevel DIR12 model for earlier Windows. This is not a claim of DIR13 isolation on Windows 10.
- The private catalog generation passes; no submission CAB is released before the final image completes the VM campaign.

## Coordinated VM qualification

The frozen private candidate completed normal and targeted Driver Verifier campaigns in an isolated Windows 10 build 19045 x64 Hyper-V guest on 2026-10-25. Its original SYS SHA-256 is `0A941B12D3B8363A559362F06EB103C348D3BCBE921CBA9F385E10742162DA31`. A temporary guest-only signature allowed testing; the host's driver/security settings were not changed.

The normal run passed **3,238 checks** and the Verifier run passed **3,240 checks**. Each included both-direction Ethernet capture, 256 independently observed raw sends, genuine Windows UDP in both directions, 1,024 receive indications, bounded ring overflow, malformed I/O rejection, genuine/forged/replayed permissions, real permission expiry and renewal, owner cleanup and an owner-open PnP restart. The fixture selected only the independent Titan Virtual Network adapter by its synthetic MAC/current module ID; it did not exercise a physical Ethernet adapter.

Restricted-token control access, administrator zero/read-only/write-only opens and a second control owner were refused. Both campaigns completed exact-package installation, runtime work, removal and service unload, with no new crash dump. The VM was restored to its original state and nine historical checkpoints. Standard Driver Verifier checks targeted only the two new drivers and the synthetic fixture; NDIS/WIFI-specific checks and low-resource simulation were not enabled.

The submission CAB preserves the exact qualified SYS/INF/CAT/PDB bytes, without recompilation; extraction round-trip hashes were checked. These are bounded engineering results, not a claim of exhaustive testing.

Microsoft-signed installation, Windows 11, Secure Boot/HVCI, representative physical adapters, other NDIS/security filters, sleep/resume, long-duration capture and throughput/latency benchmarks remain pending. A signed package will require its own exact-image installation/removal tests.

## Boundary

The filter does not request promiscuous mode or retain incoming NBLs. It copies bounded diagnostic records and forwards unmodified traffic. Original outgoing frames are passed through; owned raw sends allocate independent NBL/MDL/buffer storage and retain module lifetime until completion. Pausing rejects new ordinary sends, suppresses receive indication and drains owned pending sends before completion. Capturing is explicitly selected, one owner, default inactive and stopped on pause/detach, close or permission expiry.

The ring is finite and the capture callback is work-bounded. Its timestamps are software observation points, not physical-wire or application latency. Packet contents can be sensitive. No universal reliability, zero-overhead, perfect capture coverage or immunity to a compromised administrator/kernel is claimed.
