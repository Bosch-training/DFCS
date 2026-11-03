#include "flight_state.hpp"

// Rows D1 to D4 and D6 to D9 of DC-17 section 4.2 (SRS 10 row 7 split into D7a/D7b).
constexpr TransitionRow kTransitionTable[kTransitionRowCount] = {
    {FlightState::Boot, FlightEvent::ResetCompleted, GuardId::None, FlightState::Initializing,
     Behavior::KeepInhibited},
    {FlightState::Initializing, FlightEvent::InitPassed, GuardId::None, FlightState::Disarmed,
     Behavior::KeepInhibited},
    {FlightState::Initializing, FlightEvent::InitFailed, GuardId::None, FlightState::Fault,
     Behavior::KeepInhibitedReportFault},
    {FlightState::Disarmed, FlightEvent::ArmRequest, GuardId::Arm, FlightState::Armed,
     Behavior::EnableControlPathUnderLimits},
    {FlightState::Armed, FlightEvent::DisarmRequest, GuardId::None, FlightState::Disarmed,
     Behavior::InhibitNormalCommands},
    {FlightState::Armed, FlightEvent::CriticalFaultToFailsafe, GuardId::None, FlightState::Failsafe,
     Behavior::ApplyDefinedSafetyResponse},
    {FlightState::Armed, FlightEvent::CriticalFaultToFault, GuardId::None, FlightState::Fault,
     Behavior::ApplyDefinedSafetyResponse},
    {FlightState::Failsafe, FlightEvent::RecoveryAccepted, GuardId::Recovery, FlightState::Disarmed,
     Behavior::NeverAutoRearm},
    {FlightState::Fault, FlightEvent::ServiceReset, GuardId::ServiceReset, FlightState::Boot,
     Behavior::KeepInhibited},
};
