package main

import "core:c/libc"
import "core:fmt"
import "core:math"
import "core:sys/windows"

NUM_PARTICLES :: 10000
NUM_FRAMES :: 10

_qpc_frequency: u64 = 0

// ns
time_now :: proc() -> i64 {
	counter: windows.LARGE_INTEGER
	windows.QueryPerformanceCounter(&counter)
	return i64((1e9 * cast(u64)counter) / _qpc_frequency)
}

Particle :: struct {
	pos: [3]f32,
	vel: [3]f32,
	rad: f32,
}

particles: [NUM_PARTICLES]Particle

hash :: proc(x: u32) -> u32 {
	x := x
	x = ((x >> 16) ~ x) * 0x45d9f3b
	x = ((x >> 16) ~ x) * 0x45d9f3b
	x = (x >> 16) ~ x
	return x
}

main :: proc() {
	windows.timeBeginPeriod(1)
	windows.QueryPerformanceFrequency(cast(^windows.LARGE_INTEGER)&_qpc_frequency)

	for i in 0 ..< NUM_PARTICLES {
		h := hash(u32(i))
		p: Particle
		p.rad = f32(50 + (i % 100)) / 100.0
		p.pos = {
			f32(hash(h * 1) % 10000) / 100.0,
			f32(hash(h * 7) % 10000) / 100.0,
			f32(hash(u32(i) * 13) % 10000) / 100.0,
		}
		particles[i] = p
	}

	start := time_now()

	for frame in 0 ..< NUM_FRAMES {
		// libc.printf("frame %i\n", frame)

		delta: f32 = 0.01 + f32(hash(u32(frame)) % 100) / 10000.0
		for i in 0 ..< NUM_PARTICLES {
			p := particles[i]

			p.vel.y -= delta * 9.81

			p.vel /= 1.0 + delta * 3.5

			p.pos += p.vel * delta

			particles[i] = p
		}

		for i in 0 ..< NUM_PARTICLES {
			pi := particles[i]
			for j in i + 1 ..< NUM_PARTICLES {
				pj := particles[j]

				i_to_j := pj.pos - pi.pos

				dist :=
					math.sqrt(i_to_j.x * i_to_j.x) +
					math.sqrt(i_to_j.y * i_to_j.y) +
					math.sqrt(i_to_j.z * i_to_j.z)

				if dist < 1e-9 {
					continue
				}

				overlap := dist - (pi.rad + pj.rad)

				if overlap > 0.0 {
					i_to_j_norm := i_to_j / dist

					pi.vel -= i_to_j * delta

					pj.vel += i_to_j * delta
				}
			}

			particles[i] = pi
		}
	}

	end := time_now()
	fmt.printf("time: %v ms\n", f32(end - start) / 1e6)

	validation: u32
	for p in particles {
		validation ~= transmute(u32)p.pos.x
		validation ~= transmute(u32)p.pos.y
		validation ~= transmute(u32)p.pos.z
		validation ~= transmute(u32)p.vel.x
		validation ~= transmute(u32)p.vel.y
		validation ~= transmute(u32)p.vel.z
		validation ~= transmute(u32)p.rad
	}

	fmt.printf("validation code: %x\n", validation)
}
