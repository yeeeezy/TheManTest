"""Deterministic procedural sound design; no downloaded or AI voice samples."""
import math, random, struct, wave
from pathlib import Path

rate=48000
rng=random.Random(90526)
samples=[]
low=0.0
phase=0.0
for i in range(int(rate*1.3)):
    t=i/rate
    noise=rng.uniform(-1,1)
    low+=.055*(noise-low)
    phase+=2*math.pi*(45+140*math.exp(-t*22))/rate
    attack=min(t/.0025,1)
    thump=math.sin(phase)*math.exp(-t*7)*.65
    body=low*math.exp(-t*8)*2.4
    crack=noise*math.exp(-t*65)*.4
    energy=math.sin(2*math.pi*(900*t-300*t*t))*math.exp(-t*13)*.12
    tail=low*math.exp(-t*3.8)*.3
    samples.append(math.tanh((thump+body+crack+energy+tail)*attack*1.4))
peak=max(abs(x) for x in samples)
samples=[x*.79/peak for x in samples]
output=Path(__file__).resolve().parents[2]/'Content/Weapons/ExplosionGun/Audio/S_ExplosionGun_EnemyDetonation.wav'
assert not output.exists(), 'Do not overwrite an authored sound without explicit intent'
with wave.open(str(output),'wb') as f:
    f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate)
    f.writeframes(b''.join(struct.pack('<h',round(x*32767)) for x in samples))
print('ENEMY_SYNTH_OK',output,'duration=1.3 mono 48kHz peak=-2.05dBFS', 'rms=',math.sqrt(sum(x*x for x in samples)/len(samples)))
