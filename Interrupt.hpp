#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP


class InterruptTable { // TODO find better name
public:

private:
};

void (* const interruptVectors[])(void) __attribute__ ((used)) __attribute__ ((section (".intvecs"))) =

#endif // INTERRUPT_HPP
