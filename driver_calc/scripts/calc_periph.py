STATUS_REG_OFFSET = 0x00
OPERATION_REG_OFFSET = 0x04
DAT0_REG_OFFSET = 0x08
DAT1_REG_OFFSET = 0x0c
RESULT_REG_OFFSET = 0x10

OPERATION_ADD = (1 << 0)
OPERATION_SUB = (1 << 1)
OPERATION_MULT = (1 << 2)
OPERATION_DIV = (1 << 3)

STATUS_INVALID_OPERATION = (1 << 0)
STATUS_DIV_BY_ZERO = (1 << 1)

if request.isInit:
    status_reg = 0x00
    operation_reg = OPERATION_ADD
    dat0_reg = 0x00
    dat1_reg = 0x00
    result_reg = 0x00

elif request.isRead:

    if request.offset == STATUS_REG_OFFSET:
        request.value = status_reg
    elif request.offset == OPERATION_REG_OFFSET:
        request.value = operation_reg
    elif request.offset == DAT0_REG_OFFSET:
        request.value = dat0_reg
    elif request.offset == DAT1_REG_OFFSET:
        request.value = dat1_reg
    elif request.offset == RESULT_REG_OFFSET:
        request.value = result_reg & 0xffffffff

elif request.isWrite:

    if request.offset == STATUS_REG_OFFSET:
        status_reg = status_reg & (~request.value)
    elif request.offset == OPERATION_REG_OFFSET:
        if request.value not in [OPERATION_ADD, OPERATION_SUB, OPERATION_DIV, OPERATION_MULT]:
            status_reg = STATUS_INVALID_OPERATION
        else:
            operation_reg = request.value
            status_reg = 0
            if request.value == OPERATION_ADD:
                result_reg = dat0_reg + dat1_reg
            elif request.value == OPERATION_SUB:
                result_reg = dat0_reg - dat1_reg
            elif request.value == OPERATION_DIV:
                if dat1_reg == 0:
                    status_reg = STATUS_DIV_BY_ZERO
                else:
                    result_reg = int(dat0_reg / dat1_reg)
            elif request.value == OPERATION_MULT:
                result_reg = dat0_reg * dat1_reg
    elif request.offset == DAT0_REG_OFFSET:
        dat0_reg = request.value
    elif request.offset == DAT1_REG_OFFSET:
        dat1_reg = request.value

self.NoisyLog("%s on DUMMY at 0x%x, value 0x%x" % (str(request.type), request.offset, request.value))
