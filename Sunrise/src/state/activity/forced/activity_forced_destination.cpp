#include "activity_forced_destination.h"

#include <Windows.h>

#include <cstddef>

#include "../../runtime/storage/internal.h"

namespace sunrise::state::activity::forced {

/** Replaces the forced destination. */
bool publish(const ForcedDestination& value) noexcept {
    if (!storable(value)) {
        return false;
    }
    AcquireSRWLockExclusive(&runtime::storage::g_stateLock);
    runtime::storage::g_state.activity.forced = value;
    ReleaseSRWLockExclusive(&runtime::storage::g_stateLock);
    return true;
}

/** Copies the forced destination. */
void snapshot(ForcedDestination& value) noexcept {
    AcquireSRWLockShared(&runtime::storage::g_stateLock);
    value = runtime::storage::g_state.activity.forced;
    ReleaseSRWLockShared(&runtime::storage::g_stateLock);
}

/** Drops the selection and the switch, the same as the interface's clear action. */
void clear() noexcept {
    AcquireSRWLockExclusive(&runtime::storage::g_stateLock);
    runtime::storage::g_state.activity.forced = {};
    ReleaseSRWLockExclusive(&runtime::storage::g_stateLock);
}

/** Overwrites one committed destination with the forced one. */
bool apply(destination::DestinationSelection& selection) noexcept {
    ForcedDestination value{};
    snapshot(value);
    if (!active(value)) {
        return false;
    }

    selection.packageName = {};
    for (std::size_t index = 0; index < value.packageNameLength; ++index) {
        selection.packageName[index] = static_cast<std::int8_t>(value.packageName[index]);
    }
    selection.packageNameLength = value.packageNameLength;
    // All three are absent when a destination is forced rather than picked. Many package names
    // map to several definitions, so no index can be derived from a name.
    selection.reason = destination::kMinimumReason;
    selection.previousActivityIndex = destination::kAbsentActivityIndex;
    selection.activityIndex = destination::kAbsentActivityIndex;
    selection.elementIndex = destination::kAbsentElementIndex;
    selection.hasElementIndex = false;
    // The client named its arrival for the destination it picked, so both wire hashes go with it.
    selection.arrivalBubbleHash = 0;
    selection.hasArrivalBubbleHash = false;
    selection.spawnSetHash = 0;
    selection.hasSpawnSetHash = false;
    selection.arrivalBubbleOverride = value.bubble;
    selection.hasArrivalBubbleOverride = true;
    selection.sliceSetOverride = value.sliceSet;
    selection.hasSliceSetOverride = true;
    // With no set chosen the absent hash goes out, so the Client searches the loaded world itself.
    // A map-wide set is not proof that the arrival bubble holds one of its points.
    selection.spawnSetOverride = value.hasSpawnSetHash ? value.spawnSetHash : kAbsentSpawnSetHash;
    selection.hasSpawnSetOverride = true;
    // The captured descriptor is dropped, not patched: its activity index names what the Client
    // picked, and a forced name beside a picked index starts the wrong activity with no spawn.
    selection.descriptorBits = {};
    selection.descriptorBitLength = 0;
    return true;
}

} // namespace sunrise::state::activity::forced
