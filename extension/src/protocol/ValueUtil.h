#pragma once

namespace presonus::studiolive::gpext::protocol
{

/// GPScript linear fader percent (0..100) to PV volume scalar (0..1).
float linearPercentToVolumeScalar(double linearPercent);

/// PV volume scalar or MS percent to GPScript linear percent (0..100).
double volumeScalarToLinearPercent(double scalar);

/// GPScript pan percent (0..100, 50 = center) to PV pan scalar (0..1).
float panPercentToScalar(double panPercent);

/// PV pan scalar to GPScript pan percent.
double panScalarToPercent(double scalar);

/// GPScript dB fader (-84..+10) to linear percent (0..100), PreSonus curve.
double dbToLinearPercent(double db);

/// Linear percent (0..100) to GPScript dB (-84..+10), inverse of dbToLinearPercent.
double linearPercentToDb(double linearPercent);

} // namespace presonus::studiolive::gpext::protocol
