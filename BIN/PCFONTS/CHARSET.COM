��.�P�. �, �ڌ��.�� ��+�>��� s� �>���r(>�r"���G;�r�>� t�>� u� ;�w����� ��ډ�����+؎��JW�!_����ҋ��3�.�P���+���� �6��6���P��.�P�V ��L�F�!� ��� � 5�!�����5�!�����5�!�����5�!����� %�ʎں��!�� %���!�%���!�%���!�%���!�ô@� �!ù ���.�P���� P�V�        �R�T����� 2�&�@�Ň֓�  ��� ��� ��%����+�r`����،Ў�QI�2���݇�ًË�C� wr@� w�< t<t<	u�2����tB�
�uC��2����I,"t"<\u�<"u�I���I�YʎT��Cۋ��+�r��.���v ��6�
���t�3��F �&R�3���6���ô3ۊ6�����U��
�F� � ������]� U���F3��]� �P����
P������U��V�v�t���F�7���< u�| t�^]�U���VW3��~��~&�~u� �3����t��P�^�w�� YY�t��P�m � �b�=�3��:��P�Q�F�Ʃ u,��P�|�Y�F��F��������	�j�Y�� P�'��>�Qr��G��� r��t��P� 3�P�� Y��P� 3�_^��]�U��VW�~���F�< u���+ϸ @� ���!_^]� �U���
������������u��������v��Y]�U��>� u� ��F���㉇���3�� ]�U��VW�؎���v�~3��عza��
�t�t�:�r:�w, :�r:�w�� :�t�+�� _^]�U���ϴ5��!��%��!�f�u�~u3���Ӵ%��!�� ]�              Turbo-C -	Copyright (c) 1988 Borland Intl. Divide error
Abnormal program termination
                                       CHARSET: Display all characters of current screen font. Ver 1.5 Apr 16 1993
Copyright (c) 1990-1993 by Yossi Gil, P.O.Box 3148, Jerusalem, ISRAEL.

     Usage: charset [-p]

Will display a 16x80 screen comprising of a table of all 256 different chars
in current screen font, text demos (the quick brown fox..., figures, currency
symbols, and miscellaneous other characters), business graphic, ASCII controls,
arrows, boxes, mathematical symbols, dots, etc. CHARSET is useful for
demonstrating and examining a screen font.

FLAGS
   -p          Wait for a key hit before terminating.
   -h          Display this message.
   -H          Display this message.
   -?          Display this message.

SEE ALSO
    loadfont, withfont, palette, fontcomp
    The font collection.

COPYING POLICY Copy freely for private use. Usage as part of any commerical
  package is under the same restrictions as of Rhaul Dehsi ZOO program. If you
  cannot obtain ZOO, you cannot use 'charset' commercially. �	&
c
�
�
E���;y��.i * The quick brown fox jumped over the lazy dogs.(,.;!?:) * Misc: ����     Arrows: ��  NULL    CR    LF 
 Currency: $�����  Figures: 0123456789    BELL   BS    FF  Dots: .����     Blocks: � � � � �      TAB  	  VT   EOF  Fill: ���
       Alt-255: �             EOT   DEL   ESC  Business Graphic:                        � ������Ŀ������ͻ ������͸������ķ  Math: �x��dx � � �x����x � A �  �� B �  � � C �  �� D �  �        �          ��������� ������Ĵ������͹ ������͵������Ķ   �����   ���Ŀ   �   �   � ��������������ͼ ������;������Ľ   �����   ���۳   ���������                                     �����   ���۳   �   �   � Other drawing chars:                �����   �����   ���������  ����

 ���� ���� ���� ���� ���� ����� ���� ���� ���� ��� �1��

 �2�� �3�� �4�� �5�� �6�� � 7�� �8�� �9�� �0�� �$� ����

 ���� ���� ���� ���� ���� ����� ���� ���� ���� ��� ����

 ���� ���� ���� ���� ���� ����� ���� ���� ���� ��� -p    
Hit any key to continue... 
 ttt     1