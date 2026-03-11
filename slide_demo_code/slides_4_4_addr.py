def addr(base, i, j, N, elem_size=8):
    return base + (i * N + j) * elem_size

def trace_matmul_ijk(N, elem_size=8):
    A_base = 0
    B_base = 1_000_000
    C_base = 2_000_000
    trace = []

    for i in range(N):
        for j in range(N):
            for k in range(N):
                trace.append(addr(A_base, i, k, N, elem_size))
                trace.append(addr(B_base, k, j, N, elem_size))
                trace.append(addr(C_base, i, j, N, elem_size))
    return trace

def trace_matmul_ikj(N, elem_size=8):
    A_base = 0
    B_base = 1_000_000
    C_base = 2_000_000
    trace = []

    for i in range(N):
        for k in range(N):
            a_addr = addr(A_base, i, k, N, elem_size)
            for j in range(N):
                trace.append(a_addr)
                trace.append(addr(B_base, k, j, N, elem_size))
                trace.append(addr(C_base, i, j, N, elem_size))
    return trace