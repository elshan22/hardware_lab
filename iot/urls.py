from django.urls import path
from .views import DeviceStateView

urlpatterns = [
    path('devices/<int:device_id>/state/', DeviceStateView.as_view(), name='device-state'),
]
