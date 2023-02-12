# -*- coding: utf-8 -*-
import numpy as np
# import pandas as pd
# import matplotlib.ticker as ticker
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import scipy.optimize as spopt



# def plot_plane_for_b2():

#     #######################################

#     tmp = np.loadtxt("a2b2c2.txt")
#     a2 = tmp[0]
#     b2 = tmp[1]
#     c2 = tmp[2]

#     t_max = 10.0

#     theta_ell=np.linspace(0,2.0*np.pi,60)
#     theta_hyp1=np.linspace(-0.499*np.pi,0.499*np.pi,60)
#     theta_hyp2=np.linspace( 0.501*np.pi,1.499*np.pi,60)

#     bnd=10.0

#     n_la = 10
#     n_mu = 10
#     n_nu = 10

#     # 焦点位置, +/- Delta_2
#     f_bc = [np.sqrt(b2-c2),-np.sqrt(b2-c2)]

#     # 焦点位置, +/- Delta_1
#     f_ab = [np.sqrt(a2-b2),-np.sqrt(a2-b2)]

#     # 焦点位置, +/- sqrt(Delta_2^2+Delta_1^2)
#     f_ac = [np.sqrt(a2-c2),-np.sqrt(a2-c2)]

#     print("Plotting: a2 = %.2f b2 = %.2f c2 = %.2f" %(a2,b2,c2))

#     ######################################## 函数

#     def Equi_la_on_XY(la,theta):
#         x = np.sqrt(la-a2)*np.cos(theta)
#         y = np.sqrt(la-b2)*np.sin(theta)
#         return x,y

#     def Equi_mu_on_XY(mu,theta):
#         x = np.sqrt(a2-mu)*np.tan(theta)
#         y = np.sqrt(mu-b2)/np.cos(theta)
#         return x,y

#     #########
#     def Equi_la_on_XZ(la,theta):
#         x = np.sqrt(la-a2)*np.cos(theta)
#         z = np.sqrt(la-c2)*np.sin(theta)
#         return x,z


#     def Equi_mu_on_XZ(mu,theta):
#         x = np.sqrt(a2-mu)*np.tan(theta)
#         z = np.sqrt(mu-c2)/np.cos(theta)
#         return x,z


#     def Equi_nu_on_XZ(nu,theta):
#         x = np.sqrt(a2-nu)*np.tan(theta)
#         z = np.sqrt(nu-c2)/np.cos(theta)
#         return x,z

#     #########
#     def Equi_la_on_YZ(la,theta):
#         y = np.sqrt(la-b2)*np.cos(theta)
#         z = np.sqrt(la-c2)*np.sin(theta)
#         return y,z

#     def Equi_mu_on_YZ(mu,theta):
#         y = np.sqrt(mu-b2)*np.cos(theta)
#         z = np.sqrt(mu-c2)*np.sin(theta)
#         return y,z

#     def Equi_nu_on_YZ(nu,theta):
#         y = np.sqrt(b2-nu)*np.tan(theta)
#         z = np.sqrt(nu-c2)/np.cos(theta)
#         return y,z


#     #######################################

#     #######################################


#     '''
#     #        画图：y轴= 3个度规系数, x轴= x
#     '''
#     fig = plt.figure(figsize=(12,12))  #创建图像对象，尺寸24*24英寸
#     #fig = plt.figure(figsize=(24,13.5))  #创建图像对象，尺寸24*13.5英寸(4:3)
#     #plt.title(r"$a^2$ = %.2f $b^2$ = %.2f $c^2$ = %.2f" %(a2,b2,c2), fontsize=18)
#     '''
#     #################################### 图1 : XY
#     fig2 = fig.add_subplot(1,3,1)
#     plt.xlabel('X [kpc]',fontsize=18)
#     plt.ylabel('Y [kpc]',fontsize=18)

#     raw=np.loadtxt("CP-XY.dat")
#     t=raw[:,0]
#     x=raw[:,1]
#     y=raw[:,2]

#     if (x.max()>y.max()):
#         bnd = x.max()*1.1
#     else:
#         bnd = y.max()*1.1

#     plt.xlim([-bnd,bnd])
#     plt.ylim([-bnd,bnd])

#     plt.scatter(x,y,c=t,s=10,marker='o',cmap='rainbow')

#     #XY平面上
#     #等la线为椭圆
#     beg=np.log10(a2)
#     end=np.log10(10*a2)
#     for la in np.logspace(beg,end,n_la):
#         x,y=Equi_la_on_XY(la,theta_ell)
#         plt.plot(x,y,c='red',linestyle='--',linewidth=1)

#     #等mu线双曲线
#     beg=np.log10(b2)
#     end=np.log10(a2)
#     for mu in np.logspace(beg,end,n_mu):
#         x,y=Equi_mu_on_XY(mu,theta_hyp1)
#         plt.plot(x,y,c='blue',linestyle='--',linewidth=1)
#         x,y=Equi_mu_on_XY(mu,theta_hyp2)
#         plt.plot(x,y,c='blue',linestyle='--',linewidth=1)

#     # 焦点位置, +/- Delta_1
#     xf = np.zeros_like(f_ab)
#     yf = f_ab
#     plt.scatter(xf,yf,marker='o',s=100,c='black')

#     ################################ 图2 : XZ
#     fig2 = fig.add_subplot(1,3,2)
#     plt.xlabel('X [kpc]',fontsize=18)
#     plt.ylabel('Z [kpc]',fontsize=18)

#     raw=np.loadtxt("CP-XZ.dat")
#     t=raw[:,0]
#     x=raw[:,1]
#     z=raw[:,2]

#     if (x.max()>z.max()):
#         bnd = x.max()*1.1
#     else:
#         bnd = z.max()*1.1

#     plt.xlim([-bnd,bnd])
#     plt.ylim([-bnd,bnd])

#     plt.scatter(x,z,c=t,s=10,marker='o',cmap='rainbow')

#     #XZ平面上
#     #等 la 线为椭圆
#     beg=np.log10(a2)
#     end=np.log10(10*a2)
#     for la in np.logspace(beg,end,n_la):
#         x,z=Equi_la_on_XZ(la,theta_ell)
#         plt.plot(x,z,c='red',linestyle='--',linewidth=1)

#     #等mu 线双曲线
#     beg=np.log10(b2)
#     end=np.log10(a2)
#     for mu in np.logspace(beg,end,n_mu):
#         x,z=Equi_mu_on_XZ(mu,theta_hyp1)
#         plt.plot(x,z,c='blue',linestyle='--',linewidth=1)
#         x,z=Equi_mu_on_XZ(mu,theta_hyp2)
#         plt.plot(x,z,c='blue',linestyle='--',linewidth=1)

#     #等nu 线双曲线
#     beg=np.log10(c2)
#     end=np.log10(b2)
#     for nu in np.logspace(beg,end,n_nu):
#         x,z=Equi_nu_on_XZ(nu,theta_hyp1)
#         plt.plot(x,z,c='black',linestyle='--',linewidth=1)
#         x,z=Equi_nu_on_XZ(nu,theta_hyp2)
#         plt.plot(x,z,c='black',linestyle='--',linewidth=1)

#     # 焦点位置
#     xf = np.zeros_like(f_bc)
#     zf = f_bc
#     plt.scatter(xf,zf,marker='o',s=100,c='blue')
#     xf = np.zeros_like(f_ac)
#     zf = f_ac
#     plt.scatter(xf,zf,marker='o',s=100,c='red')
#     '''

#     #################################### 图3 : YZ
#     fig2 = fig.add_subplot(1,1,1)
#     plt.xlabel('Y [kpc]',fontsize=18)
#     plt.ylabel('Z [kpc]',fontsize=18)

#     raw=np.loadtxt("Orbit.dat")
#     t=raw[:,0]
#     y=raw[:,2]
#     z=raw[:,3]

#     if (y.max()>z.max()):
#         bnd = y.max()*1.1
#     else:
#         bnd = z.max()*1.1

#     bnd=5
#     plt.xlim([-bnd,bnd])
#     plt.ylim([-bnd,bnd])

#     plt.scatter(y,z,c=t,s=5,marker='o',cmap='rainbow')

#     #YZ平面上
#     #等 la 线为椭圆
#     beg=np.log10(a2)
#     end=np.log10(10*a2)
#     for la in np.logspace(beg,end,n_la):
#         y,z=Equi_la_on_YZ(la,theta_ell)
#         plt.plot(y,z,c='red',linestyle='--',linewidth=1)

#     #等mu 线为椭圆
#     beg=np.log10(b2)
#     end=np.log10(a2)
#     for mu in np.logspace(beg,end,n_mu):
#         y,z=Equi_mu_on_YZ(mu,theta_ell)
#         plt.plot(y,z,c='blue',linestyle='--',linewidth=1)

#     #等nu 线双曲线
#     beg=np.log10(c2)
#     end=np.log10(b2)
#     for nu in np.logspace(beg,end,n_nu):
#         y,z=Equi_nu_on_YZ(nu,theta_hyp1)
#         plt.plot(y,z,c='black',linestyle='--',linewidth=1)
#         y,z=Equi_nu_on_YZ(nu,theta_hyp2)
#         plt.plot(y,z,c='black',linestyle='--',linewidth=1)

#     # 焦点位置
#     yf = np.zeros_like(f_bc)
#     zf = f_bc
#     plt.scatter(yf,zf,marker='o',s=100,c='blue')
#     yf = np.zeros_like(f_ac)
#     zf = f_ac
#     plt.scatter(yf,zf,marker='o',s=100,c='red')
#     yf = f_ab
#     zf = np.zeros_like(f_ac)
#     plt.scatter(yf,zf,marker='o',s=100,c='black')


#     ####################################
#     plt.savefig('YZ-plane.png')
#     exit()

# def plot_plane_for_a2():

#     #######################################

#     tmp = np.loadtxt("a2b2c2.txt")
#     a2 = tmp[0]
#     b2 = tmp[1]
#     c2 = tmp[2]

#     t_max = 10.0

#     theta_ell=np.linspace(0,2.0*np.pi,60)
#     theta_hyp1=np.linspace(-0.499*np.pi,0.499*np.pi,60)
#     theta_hyp2=np.linspace( 0.501*np.pi,1.499*np.pi,60)

#     bnd=10.0

#     n_la = 10
#     n_mu = 10
#     n_nu = 10

#     # 焦点位置, +/- Delta_2
#     f_bc = [np.sqrt(b2-c2),-np.sqrt(b2-c2)]

#     # 焦点位置, +/- Delta_1
#     f_ab = [np.sqrt(a2-b2),-np.sqrt(a2-b2)]

#     # 焦点位置, +/- sqrt(Delta_2^2+Delta_1^2)
#     f_ac = [np.sqrt(a2-c2),-np.sqrt(a2-c2)]

#     print("Plotting: a2 = %.2f b2 = %.2f c2 = %.2f" %(a2,b2,c2))

#     ######################################## 函数

#     def Equi_la_on_XY(la,theta):
#         x = np.sqrt(la-a2)*np.cos(theta)
#         y = np.sqrt(la-b2)*np.sin(theta)
#         return x,y

#     def Equi_mu_on_XY(mu,theta):
#         x = np.sqrt(a2-mu)*np.tan(theta)
#         y = np.sqrt(mu-b2)/np.cos(theta)
#         return x,y

#     #########
#     def Equi_la_on_XZ(la,theta):
#         x = np.sqrt(la-a2)*np.cos(theta)
#         z = np.sqrt(la-c2)*np.sin(theta)
#         return x,z


#     def Equi_mu_on_XZ(mu,theta):
#         x = np.sqrt(a2-mu)*np.tan(theta)
#         z = np.sqrt(mu-c2)/np.cos(theta)
#         return x,z


#     def Equi_nu_on_XZ(nu,theta):
#         x = np.sqrt(a2-nu)*np.tan(theta)
#         z = np.sqrt(nu-c2)/np.cos(theta)
#         return x,z

#     #########
#     def Equi_la_on_YZ(la,theta):
#         y = np.sqrt(la-b2)*np.cos(theta)
#         z = np.sqrt(la-c2)*np.sin(theta)
#         return y,z

#     def Equi_mu_on_YZ(mu,theta):
#         y = np.sqrt(mu-b2)*np.cos(theta)
#         z = np.sqrt(mu-c2)*np.sin(theta)
#         return y,z

#     def Equi_nu_on_YZ(nu,theta):
#         y = np.sqrt(b2-nu)*np.tan(theta)
#         z = np.sqrt(nu-c2)/np.cos(theta)
#         return y,z


#     #######################################

#     #######################################


#     '''
#     #        画图：y轴= 3个度规系数, x轴= x
#     '''
#     fig = plt.figure(figsize=(12,12))  #创建图像对象，尺寸24*24英寸
#     #fig = plt.figure(figsize=(24,13.5))  #创建图像对象，尺寸24*13.5英寸(4:3)
#     #plt.title(r"$a^2$ = %.2f $b^2$ = %.2f $c^2$ = %.2f" %(a2,b2,c2), fontsize=18)

#     #################################### 图1 : XY
#     fig2 = fig.add_subplot(1,1,1)
#     plt.xlabel('X [kpc]',fontsize=18)
#     plt.ylabel('Y [kpc]',fontsize=18)

#     raw=np.loadtxt("Orbit.dat")
#     t=raw[:,0]
#     x=raw[:,1]
#     y=raw[:,2]

#     if (x.max()>y.max()):
#         bnd = x.max()*1.1
#     else:
#         bnd = y.max()*1.1

#     plt.xlim([-bnd,bnd])
#     plt.ylim([-bnd,bnd])

#     plt.scatter(x,y,c=t,s=3,marker='o',cmap='rainbow')

#     #XY平面上
#     #等la线为椭圆
#     beg=np.log10(a2)
#     end=np.log10(10*a2)
#     for la in np.logspace(beg,end,n_la):
#         x,y=Equi_la_on_XY(la,theta_ell)
#         plt.plot(x,y,c='red',linestyle='--',linewidth=1)

#     #等mu线双曲线
#     beg=np.log10(b2)
#     end=np.log10(a2)
#     for mu in np.logspace(beg,end,n_mu):
#         x,y=Equi_mu_on_XY(mu,theta_hyp1)
#         plt.plot(x,y,c='blue',linestyle='--',linewidth=1)
#         x,y=Equi_mu_on_XY(mu,theta_hyp2)
#         plt.plot(x,y,c='blue',linestyle='--',linewidth=1)

#     # 焦点位置, +/- Delta_1
#     xf = np.zeros_like(f_ab)
#     yf = f_ab
#     plt.scatter(xf,yf,marker='o',s=100,c='black')

#     '''
#     ################################ 图2 : XZ
#     fig2 = fig.add_subplot(1,3,2)
#     plt.xlabel('X [kpc]',fontsize=18)
#     plt.ylabel('Z [kpc]',fontsize=18)

#     raw=np.loadtxt("CP-XZ.dat")
#     t=raw[:,0]
#     x=raw[:,1]
#     z=raw[:,2]

#     if (x.max()>z.max()):
#         bnd = x.max()*1.1
#     else:
#         bnd = z.max()*1.1

#     plt.xlim([-bnd,bnd])
#     plt.ylim([-bnd,bnd])

#     plt.scatter(x,z,c=t,s=10,marker='o',cmap='rainbow')

#     #XZ平面上
#     #等 la 线为椭圆
#     beg=np.log10(a2)
#     end=np.log10(10*a2)
#     for la in np.logspace(beg,end,n_la):
#         x,z=Equi_la_on_XZ(la,theta_ell)
#         plt.plot(x,z,c='red',linestyle='--',linewidth=1)

#     #等mu 线双曲线
#     beg=np.log10(b2)
#     end=np.log10(a2)
#     for mu in np.logspace(beg,end,n_mu):
#         x,z=Equi_mu_on_XZ(mu,theta_hyp1)
#         plt.plot(x,z,c='blue',linestyle='--',linewidth=1)
#         x,z=Equi_mu_on_XZ(mu,theta_hyp2)
#         plt.plot(x,z,c='blue',linestyle='--',linewidth=1)

#     #等nu 线双曲线
#     beg=np.log10(c2)
#     end=np.log10(b2)
#     for nu in np.logspace(beg,end,n_nu):
#         x,z=Equi_nu_on_XZ(nu,theta_hyp1)
#         plt.plot(x,z,c='black',linestyle='--',linewidth=1)
#         x,z=Equi_nu_on_XZ(nu,theta_hyp2)
#         plt.plot(x,z,c='black',linestyle='--',linewidth=1)

#     # 焦点位置
#     xf = np.zeros_like(f_bc)
#     zf = f_bc
#     plt.scatter(xf,zf,marker='o',s=100,c='blue')
#     xf = np.zeros_like(f_ac)
#     zf = f_ac
#     plt.scatter(xf,zf,marker='o',s=100,c='red')


#     #################################### 图3 : YZ
#     fig2 = fig.add_subplot(1,1,1)
#     plt.xlabel('Y [kpc]',fontsize=18)
#     plt.ylabel('Z [kpc]',fontsize=18)

#     raw=np.loadtxt("Orbit.dat")
#     t=raw[:,0]
#     y=raw[:,2]
#     z=raw[:,3]

#     if (y.max()>z.max()):
#         bnd = y.max()*1.1
#     else:
#         bnd = z.max()*1.1

#     plt.xlim([-bnd,bnd])
#     plt.ylim([-bnd,bnd])

#     plt.scatter(y,z,c=t,s=10,marker='o',cmap='rainbow')

#     #YZ平面上
#     #等 la 线为椭圆
#     beg=np.log10(a2)
#     end=np.log10(10*a2)
#     for la in np.logspace(beg,end,n_la):
#         y,z=Equi_la_on_YZ(la,theta_ell)
#         plt.plot(y,z,c='red',linestyle='--',linewidth=1)

#     #等mu 线为椭圆
#     beg=np.log10(b2)
#     end=np.log10(a2)
#     for mu in np.logspace(beg,end,n_mu):
#         y,z=Equi_mu_on_YZ(mu,theta_ell)
#         plt.plot(y,z,c='blue',linestyle='--',linewidth=1)

#     #等nu 线双曲线
#     beg=np.log10(c2)
#     end=np.log10(b2)
#     for nu in np.logspace(beg,end,n_nu):
#         y,z=Equi_nu_on_YZ(nu,theta_hyp1)
#         plt.plot(y,z,c='black',linestyle='--',linewidth=1)
#         y,z=Equi_nu_on_YZ(nu,theta_hyp2)
#         plt.plot(y,z,c='black',linestyle='--',linewidth=1)

#     # 焦点位置
#     yf = np.zeros_like(f_bc)
#     zf = f_bc
#     plt.scatter(yf,zf,marker='o',s=100,c='blue')
#     yf = np.zeros_like(f_ac)
#     zf = f_ac
#     plt.scatter(yf,zf,marker='o',s=100,c='red')
#     yf = f_ab
#     zf = np.zeros_like(f_ac)
#     plt.scatter(yf,zf,marker='o',s=100,c='black')
#     '''

#     ####################################
#     plt.savefig('XY-plane.png')
#     exit()



def plot_orbit_3d(x, savename, is_show=False):
    fig = plt.figure()
    pointsize = 1.
    fontsize = 2.
    ax = fig.add_subplot(1,1,1, projection='3d')
    ax.grid(True)
    ax.scatter(x[:,0], x[:,1], x[:,2], s=pointsize, marker="+")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_zlabel("z")
    fig_tmp = plt.gcf()
    fig_tmp.savefig(savename+"_3d.png", format="png", dpi=300, bbox_inches='tight')
    if is_show:
        plt.show()
    plt.close("all")
    
def plot_orbit_each_2d(x, savename, is_show=False):
    fig = plt.figure()
    pointsize = 0.2
    fontsize = 2.
    xyz_name = ["x (kpc)", "y (kpc)", "z (kpc)"]
    for i in np.arange(3):
        ax = fig.add_subplot(2,2,i+1)
        ax.grid(True)
        ax.scatter(x[:,i%3], x[:,(i+1)%3], s=pointsize, marker="+")
        ax.set_xlabel(xyz_name[i%3])
        ax.set_ylabel(xyz_name[(i+1)%3])
    fig_tmp = plt.gcf()
    fig_tmp.savefig(savename+".png", format="png", dpi=300, bbox_inches='tight')
    if is_show:
        plt.show()
    plt.close("all")

def is_status_bad_orbit_ellip_at_plane(
    x, swit_axis_along, dimension=3, percent_acceptence=0.1
):
    dim0 = (swit_axis_along)%dimension
    dim1 = (swit_axis_along+1)%dimension
    dim2 = (swit_axis_along+2)%dimension
    dl0 = np.max(x[:, dim0])-np.min(x[:, dim0])
    dl1 = np.max(x[:, dim1])-np.min(x[:, dim1])
    dl2 = np.max(x[:, dim2])-np.min(x[:, dim2])
    if (dl0>np.min([dl1, dl2])*percent_acceptence):
        return 0
    else:
        return 1 

def foci_by_shape_simpleminmax(x, swit_axis_along, dimension=3):
    dim1 = (swit_axis_along+1)%dimension
    dim2 = (swit_axis_along+2)%dimension
    dl1 = np.max(x[:, dim1])-np.min(x[:, dim1])
    dl2 = np.max(x[:, dim2])-np.min(x[:, dim2])
    # print(dim1, dl1, dim2, dl2)
    D = abs(dl2**2-dl1**2)
    return D

def linear1d_to_minimize(x, a, b, y):
    return x*a+b-y

def fit_in_line(xdata1d, ydata1d):
    func = linear1d_to_minimize
    argmin = spopt.minimize(func, xdata1d, ydata1d)
    return argmin



if __name__=="__main__":

    N_grid = 32
    # N_grid = 3
    # is_show = True
    is_show = False
    write_content = []

    # for i in np.arange(N_grid):
    #     print("plot orbit_%d ..."%(i))

    #     swit = 0 #orbit rotates along x-axis (swit 0) and is at yz plane
    #     filename = "orbit_%d_b2.dat"%(i)
    #     savename = "orbit_%d_b2"%(i)
    #     data = np.loadtxt(filename) #cols: time, x, y, z, vx, vy, vz, pot
    #     x = data[:, 1:4]
    #     v = data[:, 4:7]
    #     # plot_orbit_3d(x, savename, is_show=is_show)
    #     plot_orbit_each_2d(x, savename)
    #     status_bad_yz = is_status_bad_orbit_ellip_at_plane(x, swit)
    #     D2_yz = foci_by_shape_simpleminmax(x, swit)
    #     beta = -1.-D2_yz
    #     print("status_bad_yz = %d, D2_yz = %f"%(status_bad_yz, D2_yz))

    #     swit = 2 #orbit rotates along z-axis (swit 2) and is at xy plane
    #     filename = "orbit_%d_a2.dat"%(i)
    #     savename = "orbit_%d_a2"%(i)
    #     data = np.loadtxt(filename) #cols: time, x, y, z, vx, vy, vz, pot
    #     x = data[:, 1:4]
    #     v = data[:, 4:7]
    #     # plot_orbit_3d(x, savename, is_show=is_show)
    #     plot_orbit_each_2d(x, savename)
    #     status_bad_zx = is_status_bad_orbit_ellip_at_plane(x, swit)
    #     D2_zx = foci_by_shape_simpleminmax(x, swit)
    #     alpha = beta-D2_zx
    #     print("status_bad_zx = %d, D2_zx = %f"%(status_bad_zx, D2_zx))
    #     write_content.append([0., beta, alpha, status_bad_yz, D2_yz, status_bad_zx, D2_zx])
    # filename_foci_debug = "foci_debug.txt"
    # comment_header = "# E, beta, alpha, status_bad_yz, D2_yz, status_bad_zx, D2_zx"
    # write_content = np.array(write_content)
    # np.savetxt(filename_foci_debug, np.array(write_content), header=comment_header)
    
    # # filename_foci_debug = "foci_debug.txt"
    # # write_content = np.loadtxt(filename_foci_debug)
    # # plt.plot(np.arange(N_grid), write_content[:, 1])
    # # plt.plot(np.arange(N_grid), write_content[:, 3])
    # # plt.show()



    filename_foci_debug = "../foci_example.txt"
    write_content1 = np.loadtxt(filename_foci_debug)
    filename_foci_debug = "../some_lmn_foci_Pot.txt"
    write_content2 = np.loadtxt(filename_foci_debug)
    filename_foci_debug = "foci_debug.txt"
    write_content3 = np.loadtxt(filename_foci_debug)
    y_init1 = np.linspace(0.5, 300., N_grid)
    y_init2 = write_content2[:,4]
    y_init3 = y_init2
    plt.plot(y_init1, write_content1[:,1], label="beta  by TACT_example", color="red", marker=".")
    plt.plot(y_init2, write_content2[:,1], label="beta  by do_rotate (not check)", color="green", marker=".")
    plt.plot(y_init3, write_content3[:,1], label="beta  by not_rotate", color="blue", marker=".")
    plt.plot(y_init1, write_content1[:,2], label="alpha by TACT_example", color="red", marker="+")
    plt.plot(y_init2, write_content2[:,2], label="alpha by do_rotate (not check)", color="green", marker="+")
    plt.plot(y_init3, write_content3[:,2], label="alpha by not_rotate", color="blue", marker="+")
    plt.legend()
    plt.show()
