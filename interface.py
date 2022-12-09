import random
import os
import simulate
import struct

def genKeyFile(m,key,keyFile):
    keyx = [format(key[i], 'x') for i in range(m)]
    keyHex = ""
    for i in range(m):
        if len(keyx[i]) < 2:
            keyx[i] = "0"+keyx[i]
        keyHex = keyHex+keyx[i]

    file = open(keyFile, "w")
    file.write(keyHex)
    file.close()



def runSimCPA(m,t,SNR,scoresFile,keyFile):
    r = random.randint(1000, 9999)  # random seed

    os.system(
        './src/sim-cpa -f ' + scoresFile + ' -k ' + keyFile + ' -t ' + str(t) + ' -s ' + str(SNR) + ' -r ' + str(r))
    return r


def runRank(p, keyFile, scoresFile,outputFile):

    cmd = './src/rank -f '+scoresFile+' -k '+keyFile+' -p '+str(p) +' > '+outputFile

    #print(cmd)

    os.system(cmd)

    f = open(outputFile, "r")
    content = f.read()
    data = content.split()
    #print(data)
    keyWeight = int(data[1])
    rank = int(data[3])
    ranklog2 = float(data[6])
    time = float(data[9])

    return [keyWeight,rank,ranklog2,time]

#Least Squares
def leastSquares(ptxts, leakage, m, n):
    # for each column
    results = [[0 for _ in range(n)] for _ in range(m)]
    for i in range(m):
        # for each value
        for j in range(n):
            #Generate clean bytes (trace without noise) using simulate function
            simulatedLeakage = simulate.genCleanByte(j, i, ptxts, m)
            # for each trace
            for l in range(len(ptxts)):
                #Add to the ith jth result, the diffrenece between the actual leakage and the simulated leakage squared for plaintext l
                results[i][j] += (leakage[l][i] - simulatedLeakage[l])*(leakage[l][i] - simulatedLeakage[l])
    #Find the maximum result for each column
    maxResults = [max(results[i]) for i in range(m)]
    for i in range(m):
        for j in range(n):
            #Normalising the scores
            results[i][j] = 1 - results[i][j]/maxResults[i] + 0.0001
    return results

def dpa_test(m,n,p,SNR,key,keyFile):
    ptxts = simulate.genPtxt(m, n, 100)
    traces = simulate.genTraces(key, ptxts, SNR, m)

    ro = leastSquares(ptxts, traces, m, n)

    max_ro = max([max(ro[j]) for j in range(m)])
    ro = [[max_ro - abs(ro[j][k]) for k in range(n)] for j in range(m)]

    r = []

    for i in range(m):
        for j in range(n):
            r.append(ro[i][j])

    # print(r)

    buf = struct.pack('%sd' % len(r), *r)
    # print(buf)

    scoresFile = "dpa_scores.bin"

    file2 = open(scoresFile, "wb")
    file2.write(buf)
    file2.close()

    outputFile = "rank_dpa.txt"

    [keyWeight, rank, ranklog2, time] = runRank(p, keyFile, scoresFile,outputFile)


    return [keyWeight,rank,ranklog2,time]



if __name__ == "__main__":
    m = 16
    n = 256
    t = 256
    SNR = 0.0125
    trials = 100
    p = 8

    scoresFile = 'scores.bin'

    keyFile = 'key.txt'

    key = simulate.genKey(16, 256)
    print(key)

    genKeyFile(m,key,keyFile)

    '''r = runSimCPA(m, t, SNR, scoresFile, keyFile)

    outputFile = 'rank.txt'

    [keyWeight, rank, ranklog2, time] = runRank(p, keyFile, scoresFile,outputFile)'''

    [keyWeight, rank, ranklog2, time] = dpa_test(m,n,p,SNR,key,keyFile)

    print('Key Weight: '+str(keyWeight))
    print('Rank: '+str(rank))
    print('Rank log 2: '+str(ranklog2))
    print('Time: ' +str(time))
