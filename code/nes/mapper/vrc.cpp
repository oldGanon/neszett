
static void
VrcIrq_Ctrl(cart *Cart, u8 Value)
{
    Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
    Cart->VrcIrq.IRQCtrl = Value & 7;
    if (Value & 2)
    {
        Cart->VrcIrq.IRQCounter = Cart->VrcIrq.IRQLatch;
        Cart->VrcIrq.IRQPrescaler = 341;
    }
}

static void
VrcIrq_Ack(cart *Cart)
{
    Cart->VrcIrq.IRQCtrl &= 5;
    Cart->VrcIrq.IRQCtrl |= (Cart->VrcIrq.IRQCtrl & 1) << 1;
    Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
}

static void
VrcIrq_Step(cart *Cart)
{
    if (!(Cart->VrcIrq.IRQCtrl & 2)) return;
    if (!(Cart->VrcIrq.IRQCtrl & 4))
    {
        Cart->VrcIrq.IRQPrescaler -= 3;
        if (Cart->VrcIrq.IRQPrescaler > 341)
            Cart->VrcIrq.IRQPrescaler = 341;
        else return;
    }

    if (Cart->VrcIrq.IRQCounter == 0xFF)
    {
        Cart->VrcIrq.IRQCounter = Cart->VrcIrq.IRQLatch;
        Console_SetIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
    }
    else ++Cart->VrcIrq.IRQCounter;
}
