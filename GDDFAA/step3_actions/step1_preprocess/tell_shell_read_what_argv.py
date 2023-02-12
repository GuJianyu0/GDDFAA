#!/usr/bin/env python
# -*- coding:utf-8 -*-

import numpy as np
import re

if __name__ == "__main__":

    path_file = "../../../install_and_run/user_settings.txt"
    file_handle = open(path_file, mode="r")
    char_read = file_handle.readlines()
    file_handle.close()

    argv_target_snapshots = char_read[8]
    print("argv: ", argv_target_snapshots)
    atss = argv_target_snapshots.split(" ")
    time_snapshot_init = float(atss[0])
    time_snapshot_final = float(atss[1])
    time_snapshot_target_interval = float(atss[2])
    time_snapshot_each = float(atss[3])
    snapshot_init = np.round(time_snapshot_init/time_snapshot_each) #[learn code] np.round() return float
    snapshot_final = np.round(time_snapshot_final/time_snapshot_each)
    snapshot_target_interval = np.round(time_snapshot_target_interval/time_snapshot_each)
    ts = np.array([])
    s = snapshot_init
    while s<=snapshot_final:
        ts = np.append(ts, s)
        s += snapshot_target_interval
    ts = ts.astype(int)
    N_ts = len(ts)
    # print(ts)
    path_file_w = "argv_target_snapshots.txt"
    file_handle = open(path_file_w, mode="w")
    for a in ts:
        file_handle.write("%s "%(a))
    file_handle.close()

    for i in np.arange(N_ts):
        argv_run_AA = char_read[10]
        aras = re.split(r"[ ]+", argv_run_AA)
        # aras = argv_run_AA.split(" ") #[learn code] too much splitors
        a4 = float(ts[i])*time_snapshot_each
        # print(ts[i], float(ts[i]), time_snapshot_each, a4)
        # print(str(ts[i]), str(float(ts[i])), str(time_snapshot_each), str(a4))
        a5 = np.min([float(ts[i]+0.1)*time_snapshot_each, time_snapshot_final]) #+1
        a6 = np.max([0.01, time_snapshot_each]) #d_t/d_DataTACT_load_snapshot
        a7 = time_snapshot_each #d_t/d_snapshot
        a16 = float(ts[i])*time_snapshot_each
        a17 = float(ts[i]+0.1)*time_snapshot_each
        aras[4] = "%.3f"%(a4)
        aras[5] = "%.3f"%(a5)
        aras[6] = "%.3f"%(a6)
        aras[7] = "%.3f"%(a7)
        aras[16] = "%.3f"%(a16)
        aras[17] = "%.3f"%(a17)
        print("argv: ", aras, len(aras))
        path_file_w = "argv_run_AA_%s.txt"%(ts[i])
        file_handle = open(path_file_w, mode="w")
        for a in aras:
            file_handle.write("%s "%(a))
        file_handle.close()
