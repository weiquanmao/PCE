# PCE

```txt
     ______  ______         ______  ______  _______  
    /  _  / / ____/  ___   / ____/ /  ___/ /__  __/  
   / ____/ / /___   /__/  / ___/  /___  /    / /     
  /_/     /_____/        /_____/ /_____/    /_/      
```

PCEst is a general tool for **accuracy** and **completeness** estimation of point cloud, which is designed for evaluation of reconstruction algorithms.

## Projects

* PCPre : preprocessing for ground truth model *G*.
* PCE : register reference model *R* to ground truth model *G* and estimate the minimum distances from both *R* to *G* and *G* to *R*.
* Matlab : deal with the min distances and give the accuracy and completeness evaluations, as well as the F-scores.

## Dependence

+ [Qt](www.qt.io/download/)
+ [VCG Library](http://vcg.isti.cnr.it/vcglib/) (Embedded)
+ [CPD](https://github.com/gadomski/cpd) - Coherent Point Drift

## Usage

### Step 0. Dimension Analysis of Ground Truth Model *G*

To handle the scale variation in different point cloud data, we use a relative dimensional unit *U*=0.01*lD*, where *lD* is the minimum length of the edges of the minimum enclosing box of the ground truth model *G*. And project `PCPre` is used to estimate the minimum enclosing box by principal component analysis (PCA).

### Step 1. Registration and Estimation of Minimum Distance

[**Coherent Point Drift (CPD)**](https://github.com/gadomski/cpd) is employed for registering. The ground truth model *G* and reference model *R* are first sampled, then *R* is registered to *G* with CPD, where multiple initial orientations are tested.

The project `PCE` is used for registration, besides, it also gives the the minimum distances from both *R* to *G* (`\*\_R2S.txt`) and *G* to *R* (`\*\_S2R.txt`). The definition of minimum distance from one point cloud to another point cloud can refer to [Appendix B](https://www.tanksandtemples.org/tutorial/) of online tutorial of [1].

### Step 2. Estimation of accuracy, completeness and F-scores

The estimation of accuracy, completeness and F-scores is same as [1,2], and the specific calculation details can refer to [Appendix B](https://www.tanksandtemples.org/tutorial/) of online tutorial of [1].

The Matlab program implements the final estimation with minimum distance records `\*\_R2S.txt` and `\*\_S2R.txt`. Noted that the points with distance large than *10U* are ignored as outliers.

## Examples

Estimation results of 8 space object point cloud in `TestData`.

|  Target  |    Mean    |Acc50 |Acc90 | A002 | C002 | F002 | A005 | C005 | F005 |
|:--------:|:----------:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|
| cube     |1.27(±0.93) | 0.94 | 2.49 | 74.4 | 58.7 | 65.7 | 99.8 | 82.7 | 90.4 |
| dsp      |1.19(±1.25) | 0.82 | 2.34 | 87.1 | 90.6 | 88.8 | 97.5 | 97.0 | 97.2 |
| gps      |1.66(±1.11) | 1.34 | 3.20 | 73.3 | 57.6 | 64.5 | 98.3 | 92.7 | 95.4 |
| helios   |2.28(±1.55) | 1.84 | 4.11 | 57.7 | 40.2 | 47.4 | 93.3 | 75.0 | 83.2 |
| minisat  |4.05(±2.11) | 3.91 | 7.25 | 19.8 | 12.7 | 15.5 | 72.9 | 52.3 | 60.9 |
| radarsat |3.27(±2.20) | 2.74 | 6.69 | 36.6 | 37.8 | 37.2 | 80.6 | 79.4 | 80.0 |
| scisat   |3.57(±1.97) | 3.47 | 6.14 | 30.3 | 16.1 | 21.1 | 72.0 | 49.1 | 58.4 |
| spot     |3.12(±1.86) | 2.62 | 5.60 | 33.0 | 24.8 | 28.3 | 86.5 | 71.2 | 78.1 |

Mean: average of the minimum distances;  
Acc50: the percentage of points with minimum distances within Acc50 is 50%;  
Acc90: the percentage of points with minimum distances within Acc90 is 90%;  
A002: accuracy estimation result with distance threshold *2U*;  
C002: completeness estimation result with distance threshold *2U*;  
F002: F-scores estimation result with distance threshold *2U*;  
A005: accuracy estimation result with distance threshold *5U*;  
C005: completeness estimation result with distance threshold *5U*;  
F005: F-scores estimation result with distance threshold *5U*;

The minimum distances in colors:

![Diversion](https://github.com/weiquanmao/PCE/blob/master/TestData/Diversion.jpg)

Histograms of the minimum distances:

![AccHist](https://github.com/weiquanmao/PCE/blob/master/TestData/AccHist.jpg)

> The above results can be **reproduced** by our project with data in `TestData`.

## Publications

This program and point cloud data in `TestData` have been used in the following publications:

```
@article{weiSensors18,
  author  = {Quanmao Wei and Zhiguo Jiang and Haopeng Zhang},
  title   = {Robust Spacecraft Component Detection in Point Clouds},
  journal = {Sensors},
  volume  = {18},
  year    = {2018},
  number  = {4},
  article number = {933},
  url     = {http://www.mdpi.com/1424-8220/18/4/933},
  issn    = {1424-8220},
  doi     = {10.3390/s18040933}
}
```

## References

[1] Koltun V, Koltun V, Koltun V, et al. Tanks and temples: benchmarking large-scale scene reconstruction[J]. ACM Transactions on Graphics, 2017

[2] Schops T, Schonberger J L, Galliani S, et al. A Multi-view Stereo Benchmark with High-Resolution Images and Multi-camera Videos[C]// Conference on Computer Vision and Pattern Recognition. 2017

---
By [WeiQM](https://weiquanmao.github.io) at D409.IPC.BUAA
