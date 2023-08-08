from django.db import models

class Device(models.Model):
    name = models.CharField(max_length=100)
    location = models.CharField(max_length=100)

class DeviceState(models.Model):
    device = models.ForeignKey(Device, on_delete=models.CASCADE)
    state = models.BooleanField(default=False)
    timestamp = models.DateTimeField(auto_now_add=True)
