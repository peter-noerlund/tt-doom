# TT DOOM

TT DOOM is a port of DOOM for the SoC's in Tiny Tapeout.

The plan is to at least support the following:

* Imp SoC (TTSKY26d)
* TinyQV Asteroids (TTSKY25a)

## Optimization history

| Commit     | Demo 1 | Demo 2 | Demo 3 | Description                                                     |
|------------|-------:|-------:|-------:|-----------------------------------------------------------------|
| `a71bb6f3` |  0.132 |  0.129 |  0.130 | Reference with no optimization at all                           |
| `10691adf` |  0.246 |  0.235 |  0.237 | Use DOOM palette direcly, bypassing DOOM generic RGB conversion |
| `6a765002` |  0.310 |  0.317 |  0.302 | Optimization of video encoding using looktables and a fast path |
| `4d12516e` |  0.347 |  0.354 |  0.345 | Optimized version of `FixedMul`                                 |
| `b1629cc1` |  0.365 |  0.370 |  0.359 | Optimized version of `__mulsi3`                                 | 
| `07cd14ad` |  0.401 |  0.406 |  0.393 | No global variables in draw loops                               |
| `bea0501b` |  0.450 |  0.460 |  0.446 | Do not render invisible columns                                 |
| `afca50e8` |  0.462 |  0.470 |  0.458 | Faster memcpy/mmeset                                            |
