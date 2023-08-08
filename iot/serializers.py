from rest_framework import serializers
from .models import DeviceState

class DeviceStateSerializer(serializers.ModelSerializer):
    class Meta:
        model = DeviceState
        fields = '__all__'
