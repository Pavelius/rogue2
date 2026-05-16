///////////////////////////////////////////////////////////////////////////
// 
//  Copyright 2026 by Pavel Chistyakov
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "direction.h"

directionn round(directionn d, directionn v) {
	switch(v) {
	case NorthEast:
		switch(d) {
		case North: return NorthEast;
		case NorthEast: return East;
		case East: return SouthEast;
		case SouthEast: return South;
		case South: return SouthWest;
		case SouthWest: return West;
		case West: return NorthWest;
		case NorthWest: return North;
		default: return d;
		}
		break;
	case NorthWest:
		switch(d) {
		case North: return NorthWest;
		case NorthWest: return West;
		case West: return SouthWest;
		case SouthWest: return South;
		case South: return SouthEast;
		case SouthEast: return East;
		case East: return NorthEast;
		case NorthEast: return North;
		default: return d;
		}
		break;
	case East:
		switch(d) {
		case North: return East;
		case East: return South;
		case South: return West;
		case West: return North;
		default: return d;
		}
		break;
	case West:
		switch(d) {
		case North: return West;
		case West: return South;
		case South: return East;
		case East: return North;
		default: return d;
		}
		break;
	case South:
		switch(d) {
		case North: return South;
		case West: return East;
		case South: return North;
		case East: return West;
		default: return d;
		}
		break;
	default:
		return d;
	}
}