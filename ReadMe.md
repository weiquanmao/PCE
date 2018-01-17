# PCE

```txt
     ______  ______         ______  ______  _______  
    /  _  / / ____/  ___   / ____/ /  ___/ /__  __/  
   / ____/ / /___   /__/  / ___/  /___  /    / /     
  /_/     /_____/        /_____/ /_____/    /_/      
```

PCEst is a general tool for **accuracy** and **completeness** estimation of point cloud, which is designed for evaluation of reconstruction algorithms.

## Dependence

+ [Qt](www.qt.io/download/)
+ [VCGLib](https://github.com/cdcseacave/VCG) (Embedded)
+ [CPD](https://github.com/gadomski/cpd) - Coherent Point Drift


[**Coherent Point Drift (CPD)**](https://github.com/gadomski/cpd) is employed for registering. The ground truth model *G* and reference model *R* are first sampled to 1K, then *R* is registered to *G* with CPD, where multiple initial orientations are tested.