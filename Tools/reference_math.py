import json,struct,argparse
from pathlib import Path
import numpy as np
from scipy.signal import lfilter
from scipy.optimize import least_squares
def read(path):
    b=Path(path).read_bytes(); i=12
    while i+8<=len(b):
        k,n=struct.unpack_from('<4sI',b,i)
        if k==b'data':
            u=np.frombuffer(b[i+8:i+8+n],np.uint8).reshape(-1,3).astype(np.int32)
            return ((u[:,0]|u[:,1]<<8|u[:,2]<<16)^0x800000).astype(np.float64)/8388608-1.
        i+=8+n+(n&1)
    raise ValueError('No audio')
def shelf(fc,db,d=np.sqrt(2)):
    k=np.tan(np.pi*fc/48000); v=10**(db/20); n=1/(1+d*k+k*k)
    return np.array([v+np.sqrt(v)*d*k+k*k,2*(k*k-v),v-np.sqrt(v)*d*k+k*k])*n,np.array([1,2*(k*k-1)*n,(1-d*k+k*k)*n])
def model(x,p,family='general'):
    g,k,q,fc,db,postfc,postdb=p[:7]
    d1,d2=p[7:9] if len(p)>=9 else (np.sqrt(2),np.sqrt(2))
    tail,memory=p[9:11] if len(p)==11 else (1.,0.)
    b,a=shelf(fc,db,d1); u=lfilter(b,a,x)
    if family=='general':
        bounded=np.clip(u,-32.,32.)
        shaped=g*bounded/(1+np.abs(k*bounded)**q)**(tail/q)
    elif family=='tanh': shaped=g*np.tanh(k*u)/k
    elif family=='atan': shaped=g*np.arctan(k*u)/k
    else: raise ValueError('Unknown curve family')
    previous=np.concatenate(([0.],shaped[:-1]))
    corrected=shaped+memory*(u*u/(1+u*u))*(shaped-previous)
    postb,posta=shelf(postfc,postdb,d2)
    return lfilter(postb,posta,corrected)

