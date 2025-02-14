#ifndef DEEP_SLEEP_H
#define DEEP_SLEEP_H

void deep_sleep_duration(double duration);
void deep_sleep_interrupt(int gpio);
void deep_sleep_forever();

#endif // DEEP_SLEEP_H