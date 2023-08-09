from django.db import models
from rest_framework.exceptions import ValidationError


def password_validation(password):
    if len(password) < 8:
        return ValidationError('password must be at least 8 characters long!')
    if password.isdigit() or password.isalpha():
        return ValidationError('password must contain at least 1 letter and 1 number!')


class User(models.Model):

    username = models.CharField(max_length=30, unique=True)
    password = models.CharField(max_length=30, validators=[password_validation])
    email = models.EmailField(unique=True, null=True)
    is_admin = models.BooleanField()
