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
* PCE : register reference model *R* to ground truth model *G* and estimate the min distances from both *R* to *G* and *G* to *R*.
* Matlab : deal with the min distances and give the accuracy and completeness evaluations, as well as the F-scores.

## Dependence

+ [Qt](www.qt.io/download/)
+ [VCGLib](https://github.com/cdcseacave/VCG) (Embedded)
+ [CPD](https://github.com/gadomski/cpd) - Coherent Point Drift


[**Coherent Point Drift (CPD)**](https://github.com/gadomski/cpd) is employed for registering. The ground truth model *G* and reference model *R* are first sampled to 1K, then *R* is registered to *G* with CPD, where multiple initial orientations are tested.